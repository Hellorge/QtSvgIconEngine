#include "SvgIconEngine.h"
#include <QFile>
#include <QColor>
#include <QPainter>
#include <QSvgRenderer>
#include <QApplication>
#include <QDebug>

SvgIconEngine::SvgIconEngine(const QString &iconPath) {
    this->iconPath = iconPath;
    qDebug() << iconPath;
}

SvgIconEngine::~SvgIconEngine() {}

QIcon SvgIconEngine::getIcon(const QString &style, const QString &iconName, const QVariantMap &colors) {
    QString filePath = QString("%1/%2/%3.svg").arg(iconPath).arg(style).arg(iconName);
    QPixmap pixmap = createPixmap(filePath, colors);
    return QIcon(pixmap);
}

QPixmap SvgIconEngine::createPixmap(const QString &filePath, const QVariantMap &colors) {
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

    if (!colors.isEmpty()) {
        QPixmap coloredPixmap(size);
        coloredPixmap.fill(Qt::transparent);

        QPainter colorPainter(&coloredPixmap);
        renderer.render(&colorPainter);
        colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        colorPainter.fillRect(coloredPixmap.rect(), colors.value("default").value<QColor>());
        colorPainter.end();

        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.drawPixmap(0, 0, coloredPixmap);
    }

    painter.end();
    return pixmap;
}
