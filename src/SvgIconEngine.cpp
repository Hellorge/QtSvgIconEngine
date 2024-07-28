#include "SvgIconEngine.h"
#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QMutexLocker>

Q_LOGGING_CATEGORY(lcSvgIconEngine, "svgiconengine")

SvgIconEngine::SvgIconEngine(const QString &filePath, const QVariantMap &options)
    : iconPath(filePath), cachePolicy(CachePolicy::LRU) {

	setDefaults(options);
	setCachePolicy(cachePolicy);

	// loadCacheFromDisk();
	// connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &SvgIconEngine::saveCacheToDisk);
}

SvgIconEngine::~SvgIconEngine() {
	// saveCacheToDisk();
}

void SvgIconEngine::setDefaults(const QVariantMap &options) {
    defOptions.insert("color", QApplication::palette().text().color());
    defOptions.insert("background", QColor(Qt::transparent));
    defOptions.insert("default_colors", false);
    defOptions.insert("opacity", 1.0);
    defOptions.insert("border_color", QApplication::palette().text().color());
    defOptions.insert("border_width", 0);
    defOptions.insert("shadow", false);

    for (auto it = defOptions.constBegin(); it != defOptions.constEnd(); ++it) {
        QString property = it.key();

		if (options.contains(property)) {
			if (options.value(property).isValid()) {
				defOptions.insert(property, options.value(property));
			} else {
				qCWarning(lcSvgIconEngine) << "SvgIconEngine Error: Value for option" << property << "is invalild";;
			}
		}
    }

    if (options.contains("size") && options.value("size").isValid()) {
        defOptions.insert("size", options.value("size"));
    }
}

SvgIcon* SvgIconEngine::getIcon(const QString &style, const QString &iconName) {
    QVariantMap options;
    return getIcon(style, iconName, options);
}

SvgIcon* SvgIconEngine::getIcon(const QString &style, const QString &iconName, QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);

    QSvgRenderer *renderer = getRenderer(filePath);

    if (!renderer || !renderer->isValid()) {
        qCWarning(lcSvgIconEngine) << "Failed to get a valid QSvgRenderer for" << filePath;
        // return drawNullIcon();
    }

    options = getOptions(renderer, options);

    return new SvgIcon(renderer, options);
}

QVariantMap SvgIconEngine::getOptions(const QSvgRenderer *renderer, QVariantMap &options) {
    if (!defOptions.contains("size") || !defOptions.value("size").isValid()) {
        options.insert("size", renderer->defaultSize());
    }

	for (auto it = defOptions.constBegin(); it != defOptions.constEnd(); ++it) {
        QString property = it.key();

        if (!options.contains(property) || !options.value(property).isValid()) {
			options.insert(property, defOptions.value(property));
		}
    }

    return options;
}

QSvgRenderer* SvgIconEngine::getRenderer(const QString &filePath) {
    QSvgRenderer *renderer = nullptr;

	if (cachePolicy != CachePolicy::NONE && rendererCache.contains(filePath)) {
        renderer = rendererCache.object(filePath);
        if (renderer && renderer->isValid()) {
	        return renderer;
	    }
	}

    renderer = new QSvgRenderer(filePath);
    if (!renderer->isValid()) {
        qCCritical(lcSvgIconEngine) << "Failed to create a valid QSvgRenderer for" << filePath;
        delete renderer;
        return nullptr;
    }

    if (cachePolicy != CachePolicy::NONE) {
        rendererCache.insert(filePath, renderer);
    }

    return renderer;
}

QIcon SvgIconEngine::drawNullIcon() {
    static QPen pen(defOptions.value("color").value<QColor>(), 8);

    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(pen);
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    painter.drawLine(0, 0, pixmap.width() - 1, pixmap.height() - 1);
    painter.drawLine(0, pixmap.height() - 1, pixmap.width() - 1, 0);
    painter.end();

    return QIcon(pixmap);
}

void SvgIconEngine::loadIconAsync(const QString &filePath) {
	// static QVector<QFuture<void>> futures;

	// futures.erase(
    //     std::remove_if(
    //         futures.begin(),
    //         futures.end(),
    //         [](const QFuture<void> &future) {
    //             return future.isFinished();
    //         }
    //     ),
    //     futures.end()
    // );

    // QFuture<void> future = QtConcurrent::run([this, filePath]() {
    //     QPixmap pixmap = getPixmap(filePath);
    // });

    // futures.append(future);
}

void SvgIconEngine::logError(const QString &message) {
    qCCritical(lcSvgIconEngine) << "SvgIconEngine Error:" << message;

    // QFile logFile("SvgIconEngine.log");
    // if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
    //     QTextStream stream(&logFile);
    //     stream << QDateTime::currentDateTime().toString() << ": " << message << "\n";
    //     logFile.close();
    // }
}

void SvgIconEngine::clearCache() {
    rendererCache.clear();
}

void SvgIconEngine::setCachePolicy(CachePolicy policy) {
    cachePolicy = policy;
    if (policy == CachePolicy::ALL) {
        rendererCache.setMaxCost(INT_MAX);
    } else {
        rendererCache.setMaxCost(100);
    }
}

// Uncomment and implement as needed
// QString SvgIconEngine::getCacheDirectory() {
//     QString appName = QApplication::applicationName();
//     QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
//     return cacheDir + "/" + appName;
// }

// void SvgIconEngine::saveCacheToDisk() {
//     QMutexLocker locker(&cacheMutex);

//     if (cachePolicy == CachePolicy::NONE) {
//         return;
//     }

//     QString cacheDir = getCacheDirectory();
//     QString cachePath = cacheDir + "/SvgIconEngineCache.dat";

//     QDir dir;
//     if (!dir.exists(cacheDir)) {
//         dir.mkpath(cacheDir);
//     }

//     QFile file(cachePath);
//     if (file.open(QIODevice::WriteOnly)) {
//         QDataStream out(&file);

//         int cacheSize = (cachePolicy == CachePolicy::ALL) ? rendererCache.size() : rendererCache.maxCost();
//         out << cacheSize;
//         int count = 0;
//         for (const QString &key : rendererCache.keys()) {
//             if (count >= cacheSize) {
//                 break;
//             }
//             out << key;

//             QSvgRenderer *renderer = rendererCache.object(key);
//             QByteArray svgData;
//             QBuffer buffer(&svgData);
//             buffer.open(QIODevice::WriteOnly);
//             renderer->render(&buffer);

//             out << svgData;
//             ++count;
//         }
//         file.close();
//     } else {
//         qCCritical(lcSvgIconEngine) << "Failed to open cache file for writing:" << cachePath;
//     }
// }

// void SvgIconEngine::loadCacheFromDisk() {
//     QMutexLocker locker(&cacheMutex);

//     if (cachePolicy == CachePolicy::NONE) {
//         return;
//     }

//     QString cacheDir = getCacheDirectory();
//     QString cachePath = cacheDir + "/SvgIconEngineCache.dat";

//     QFile file(cachePath);
//     if (file.open(QIODevice::ReadOnly)) {
//         QDataStream in(&file);

//         int cacheSize;
//         in >> cacheSize;

//         int maxLoadSize = (cachePolicy == CachePolicy::ALL) ? cacheSize : qMin(cacheSize, rendererCache.maxCost());
//         for (int i = 0; i < maxLoadSize; ++i) {
//             QString key;
//             QByteArray svgData;

//             in >> key;
//             in >> svgData;

//             QSvgRenderer *renderer = new QSvgRenderer;
//             QBuffer buffer(&svgData);
//             buffer.open(QIODevice::ReadOnly);
//             renderer->load(&buffer);

//             rendererCache.insert(key, renderer);
//         }
//         file.close();
//     } else {
//         qCCritical(lcSvgIconEngine) << "Failed to open cache file for reading:" << cachePath;
//     }
// }
