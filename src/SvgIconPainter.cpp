// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIconPainter.h"

#include <QPainter>
#include <QPen>
#include <QSvgRenderer>

namespace SvgIconPainter {

QImage rasterize(QSvgRenderer *renderer, const QSize &logicalSize, qreal dpr,
                 const QString &elementId) {
    if (!renderer || !renderer->isValid() || logicalSize.isEmpty() || dpr <= 0.0)
        return QImage();

    QImage image(logicalSize * dpr, QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(dpr);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // Render into an explicit logical-coordinate rect. The painter is already
    // scaled by the image's devicePixelRatio, but QSvgRenderer::render(painter)
    // targets painter->viewport(), which is in *device* pixels — at dpr 2 that
    // draws the artwork twice as large and clips it to its top-left quarter.
    const QRectF bounds(QPointF(0, 0), QSizeF(logicalSize));

    // Sprites render a single element into the same bitmap, so tinting, opacity
    // and borders apply to them exactly as they do to whole files.
    if (!elementId.isEmpty())
        renderer->render(&painter, elementId, bounds);
    else
        renderer->render(&painter, bounds);

    return image;
}

void composite(QPainter *painter, const QRect &rect, const QImage &uncolored,
               const QVariantMap &o) {
    if (!painter || uncolored.isNull() || rect.isEmpty())
        return;

    const QColor background  = o.value(QStringLiteral("background")).value<QColor>();
    const QColor color       = o.value(QStringLiteral("color")).value<QColor>();
    const bool defaultColors = o.value(QStringLiteral("default_colors")).toBool();
    const qreal opacity      = o.value(QStringLiteral("opacity"), 1.0).toReal();
    const qreal scale        = o.value(QStringLiteral("scale"), 1.0).toReal();
    const QColor borderColor = o.value(QStringLiteral("border_color")).value<QColor>();
    const qreal borderWidth  = o.value(QStringLiteral("border_width"), 0.0).toReal();

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

    if (background.alpha() > 0)
        painter->fillRect(rect, background);

    QImage artwork = uncolored;
    if (!defaultColors && color.alpha() > 0) {
        // SourceIn keeps the artwork's alpha and replaces its colour, which is
        // what makes monochrome tinting work on arbitrary SVGs.
        QPainter tint(&artwork);
        tint.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tint.fillRect(artwork.rect(), color);
    }

    painter->setOpacity(opacity);
    const QSizeF scaled(rect.width() * scale, rect.height() * scale);
    const QRectF target(rect.x() + (rect.width()  - scaled.width())  / 2.0,
                        rect.y() + (rect.height() - scaled.height()) / 2.0,
                        scaled.width(), scaled.height());
    painter->drawImage(target, artwork);

    if (borderWidth > 0) {
        painter->setOpacity(1.0); // border always fully opaque
        QPen borderPen(borderColor, borderWidth);
        borderPen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(borderPen);
        painter->setBrush(Qt::NoBrush);
        // inset by half the width so the stroke doesn't clip at the edges
        const qreal half = borderWidth / 2.0;
        painter->drawRect(QRectF(rect).adjusted(half, half, -half, -half));
    }

    painter->restore();
}

} // namespace SvgIconPainter
