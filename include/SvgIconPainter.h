#ifndef SVGICONPAINTER_H
#define SVGICONPAINTER_H

#include <QImage>
#include <QRect>
#include <QVariantMap>

class QPainter;
class QSvgRenderer;

// Shared rendering used by both SvgIcon (the animatable widget) and
// SvgQIconEngine (the QIcon backend), so the two can never drift apart.
namespace SvgIconPainter {

// Rasterise an SVG — or one element of a sprite — into a transparent image of
// `logicalSize`, backed at `dpr` device pixels per logical pixel. No colouring
// is applied; tinting happens in composite(), which lets callers cache this.
QImage rasterize(QSvgRenderer *renderer, const QSize &logicalSize, qreal dpr,
                 const QString &elementId = QString());

// Draw background, tinted artwork and border into `rect`, using an already
// resolved option map (see SvgIcon::resolveOptions).
void composite(QPainter *painter, const QRect &rect, const QImage &uncolored,
               const QVariantMap &resolved);

} // namespace SvgIconPainter

#endif // SVGICONPAINTER_H
