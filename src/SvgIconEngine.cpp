#include "SvgIconEngine.h"
#include <QColor>
#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMutexLocker>

SvgIconEngine::SvgIconEngine(const QString &filePath, const QVariantMap &options)
	: iconPath(filePath), cachePolicy(CachePolicy::LRU)
{
	iconProperties << "color" << "background" << "default_colors";

	setDefaults(options);
	setCachePolicy(cachePolicy);

	loadCacheFromDisk();
	connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &SvgIconEngine::saveCacheToDisk);
}

SvgIconEngine::~SvgIconEngine() {
	saveCacheToDisk();
}

void SvgIconEngine::setDefaults(const QVariantMap &options) {
	defOptions = options;

	if (!defOptions.contains("color") || !defOptions.value("color").isValid()) {
        defOptions.insert("color", QApplication::palette().text().color());
    }

   	if (!defOptions.contains("background") || !defOptions.value("background").isValid()) {
        defOptions.insert("background", QColor(Qt::transparent));
    }

   	if (!defOptions.contains("default_colors")) {
        defOptions.insert("default_colors", false);
    }
}

void SvgIconEngine::clearCache() {
    pixmapCache.clear();
}

void SvgIconEngine::setCachePolicy(CachePolicy policy) {
    cachePolicy = policy;
    if (policy == CachePolicy::ALL) {
        pixmapCache.setMaxCost(INT_MAX);
    } else {
        pixmapCache.setMaxCost(100);
    }
}

QIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = getPixmap(filePath);

    pixmap = applyOptions(pixmap, options);

    return SvgIcon(pixmap);
}

QPixmap SvgIconEngine::getPixmap(const QString &filePath) {
	if (cachePolicy != CachePolicy::NONE && pixmapCache.contains(filePath)) {
		return *pixmapCache.object(filePath);
	}

    return createPixmap(filePath);
}

QPixmap SvgIconEngine::createPixmap(const QString &filePath) {
	QSvgRenderer renderer(filePath);

	if (!renderer.isValid()) {
        return drawNullIcon();
    }

    QSize size = renderer.defaultSize();
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    if (cachePolicy != CachePolicy::NONE) {
    	pixmapCache.insert(filePath, new QPixmap(pixmap));
    }

    return pixmap;
}

QPixmap SvgIconEngine::applyOptions(QPixmap pixmap, const QVariantMap &options) {
	auto getOption = [&](const QString &key) {
        return options.value(key, defOptions.value(key));
    };

    if (!getOption("default_colors").toBool()) {
    	QPixmap bgPixmap(pixmap.size());
		bgPixmap.fill(getOption("background").value<QColor>());

		QPainter painter(&bgPixmap);
	    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

		{
			QPainter tempPainter(&pixmap);
			tempPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
		    tempPainter.fillRect(pixmap.rect(), getOption("color").value<QColor>());
		    tempPainter.end();
		}

		painter.drawPixmap(0, 0, pixmap);
	    painter.end();

		return bgPixmap;
    }

    return pixmap;
}

QPixmap SvgIconEngine::drawNullIcon() {
    static QPen pen(defOptions.value("color").value<QColor>(), 8);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(pen);
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    painter.drawLine(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    painter.drawLine(0, pixmap.height() - 1, pixmap.width() - 1, 0);
    painter.end();

    return pixmap;
}

void SvgIconEngine::loadIconAsync(const QString &filePath) {
	// static QVector<QFuture<void>> futures;

	// futures.erase(
	// 	std::remove_if(
	// 	  	futures.begin(),
	// 	   	futures.end(),
	// 		[](const QFuture<void> &future) {
	// 		   	return future.isFinished();
	// 		}
	// 	),
	// 	futures.end()
 //    );

 //    QFuture<void> future = QtConcurrent::run([this, filePath]() {
 //        QPixmap pixmap = getPixmap(filePath);
 //    });

 //    futures.append(future);
}

void SvgIconEngine::logError(const QString &message) {
    qWarning() << "SvgIconEngine Error:" << message;

    // QFile logFile("SvgIconEngine.log");
    // if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    //     QTextStream stream(&logFile);
    //     stream << QDateTime::currentDateTime().toString() << ": " << message << "\n";
    //     logFile.close();
    // }
}

QString getCacheDirectory() {
    QString appName = QApplication::applicationName();
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + "/" + appName;
}

void SvgIconEngine::saveCacheToDisk() {
	QMutexLocker locker(&cacheMutex);

	if (cachePolicy == CachePolicy::NONE) {
		return;
	}

	QString cacheDir = getCacheDirectory();
    QString cachePath = cacheDir + "/SvgIconEngineCache.dat";

    QDir dir;
    if (!dir.exists(cacheDir)) {
        dir.mkpath(cacheDir);
    }

    QFile file(cachePath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);

        int cacheSize = (cachePolicy == CachePolicy::ALL) ? pixmapCache.size() : pixmapCache.maxCost();
        out << cacheSize;
        int count = 0;
        foreach (const QString &key, pixmapCache.keys()) {
            if (count >= cacheSize) {
                break;
            }
            out << key;
            out << *(pixmapCache.object(key));
            ++count;
        }
        file.close();
    } else {
        logError("Failed to open cache file for writing: " + cachePath);
    }
}

void SvgIconEngine::loadCacheFromDisk() {
	QMutexLocker locker(&cacheMutex);

	if (cachePolicy == CachePolicy::NONE) {
		return;
	}

	QString cacheDir = getCacheDirectory();
    QString cachePath = cacheDir + "/SvgIconEngineCache.dat";

    QFile file(cachePath);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);

        int cacheSize;
        in >> cacheSize;

        int maxLoadSize = (cachePolicy == CachePolicy::ALL) ? cacheSize : qMin(cacheSize, pixmapCache.maxCost());
        for (int i = 0; i < maxLoadSize; ++i) {
            QString key;
            QPixmap pixmap;
            in >> key;
            in >> pixmap;
            pixmapCache.insert(key, new QPixmap(pixmap));
        }
        file.close();
    } else {
        logError("Failed to open cache file for reading: " + cachePath);
    }
}
