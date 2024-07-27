#include "SvgIconEngine.h"
#include <QColor>
#include <QDebug>
#include <QApplication>

SvgIconEngine::SvgIconEngine(const QString &filePath, const QVariantMap &options)
	: iconPath(filePath), pixmapCache(100)
{
	setDefaults(options);
}

SvgIconEngine::~SvgIconEngine()
{}

void SvgIconEngine::setDefaults(const QVariantMap &options) {
	defOptions = options;

	if (!defOptions.contains("color")) {
        defOptions.insert("color", QApplication::palette().text().color());
    }

   	if (!defOptions.contains("default_colors")) {
        defOptions.insert("default_colors", false);
    }
}

void SvgIconEngine::clearCache() {
    pixmapCache.clear();
}

QIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = getPixmap(filePath);

    pixmap = applyOptions(pixmap, options);

    return SvgIcon(pixmap);
}

QPixmap SvgIconEngine::getPixmap(const QString &filePath) {
	if (pixmapCache.contains(filePath)) {
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

    pixmapCache.insert(filePath, new QPixmap(pixmap));

    return pixmap;
}

QPixmap SvgIconEngine::applyOptions(QPixmap pixmap, const QVariantMap &iconOptions) {
	QVariantMap options = defOptions;

	if (iconOptions.contains("color") && iconOptions.value("color").isValid()) {
        options.insert("color", iconOptions.value("color"));
    }

    if (iconOptions.contains("default_colors")) {
    	options.insert("default_colors", iconOptions.value("default_colors"));
    }

    if (!options.value("default_colors").toBool()) {
	    QPainter painter(&pixmap);
	    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	    painter.fillRect(pixmap.rect(), options.value("color").value<QColor>());
	    painter.end();
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
    // QtConcurrent::run([=]() {
    //     getPixmap(filePath);
    // });
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
