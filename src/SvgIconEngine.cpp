#include "SvgIconEngine.h"
#include <QColor>
#include <QDebug>

SvgIconEngine::SvgIconEngine(const QString &filePath) : iconPath(filePath) {}

SvgIconEngine::~SvgIconEngine() {}

SvgIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &options) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = createPixmap(filePath, options);
    return SvgIcon(pixmap);
}

QPixmap SvgIconEngine::createPixmap(const QString &filePath, const QVariantMap &options) {
    QSvgRenderer renderer(filePath);
    if (!renderer.isValid()) {
        qWarning() << "SVG file does not exist or is invalid:" << filePath;
        return QPixmap();
    }

    QSize size = renderer.defaultSize();

    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);

    if (!options.isEmpty() && options.value("default").isValid()) {
        QPixmap coloredPixmap(size);
        coloredPixmap.fill(Qt::transparent);

        QPainter colorPainter(&coloredPixmap);
        renderer.render(&colorPainter);
        colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        colorPainter.fillRect(coloredPixmap.rect(), options.value("default").value<QColor>());
        colorPainter.end();

        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawPixmap(0, 0, coloredPixmap);
    }

    painter.end();
    return pixmap;
}