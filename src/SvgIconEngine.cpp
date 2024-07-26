#include "SvgIconEngine.h"
#include <QColor>
#include <QDebug>
#include <QApplication>

SvgIconEngine::SvgIconEngine(const QString &filePath) : iconPath(filePath), defIconColor(QApplication::palette().text().color()) {}

SvgIconEngine::~SvgIconEngine() {}

SvgIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = createPixmap(filePath, options);

    return SvgIcon(pixmap);
}

QPixmap SvgIconEngine::createPixmap(const QString &filePath, const QVariantMap &options) {

	QSvgRenderer renderer(filePath);

	if (!renderer.isValid()) {
        return drawNullIcon();
    }

    QSize size = renderer.defaultSize();

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);

    QColor iconColor = defIconColor;

    if (options.value("color").isValid()) {
		iconColor = options.value("color").value<QColor>();
    }

    if (!options.value("default_colors").value<bool>()) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), iconColor);
    }

    painter.end();

    return pixmap;
}

QPixmap SvgIconEngine::drawNullIcon() {
    static QPen pen(QApplication::palette().text().color(), 8);

   	// pen.setWidth(8);
    // pen.setColor(QApplication::palette().text().color());

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
