#include "SvgIconEngine.h"
#include <QColor>
#include <QDebug>
#include <QApplication>

SvgIconEngine::SvgIconEngine(const QString &filePath) : iconPath(filePath), defIconColor(QApplication::palette().text().color()) {}

SvgIconEngine::~SvgIconEngine() {}

QIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = getPixmap(filePath);

    pixmap = applyOptions(pixmap, options);

    return SvgIcon(pixmap);
}

QPixmap SvgIconEngine::getPixmap(const QString &filePath) {
	if (pixmapCache.contains(filePath)) {
		return pixmapCache.value(filePath);
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

    pixmapCache.insert(filePath, pixmap);

    return pixmap;
}

QPixmap SvgIconEngine::applyOptions(QPixmap pixmap, const QVariantMap &options) {
	QColor iconColor = defIconColor;

    if (options.value("color").isValid()) {
    	iconColor = options.value("color").value<QColor>();
    }

    if (!options.value("default_colors").value<bool>()) {
	    QPainter painter(&pixmap);
	    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	    painter.fillRect(pixmap.rect(), iconColor);
	    painter.end();
    }

    return pixmap;
}

QPixmap SvgIconEngine::drawNullIcon() {
    static QPen pen(QApplication::palette().text().color(), 8);

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
