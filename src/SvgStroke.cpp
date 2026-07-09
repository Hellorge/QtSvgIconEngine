// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgStroke.h"

#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

namespace SvgStroke {

namespace {

// Upper bound for the bisection. Icon viewBoxes are typically 512 units, so a
// subpath longer than this doesn't occur in practice; hitting the cap means the
// artwork has no stroke at all.
constexpr qreal kMaxLength = 16384.0;
constexpr int   kProbeSize = 64;      // px; big enough to see a hairline stroke
constexpr int   kBisectSteps = 18;    // 16384 / 2^18 -> sub-unit precision

int strokeCoverage(const QByteArray &svg) {
    QSvgRenderer renderer(svg);
    if (!renderer.isValid())
        return -1;

    QImage image(kProbeSize, kProbeSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    {
        QPainter painter(&image);
        renderer.render(&painter, QRectF(0, 0, kProbeSize, kProbeSize));
    }

    int covered = 0;
    for (int y = 0; y < kProbeSize; ++y)
        for (int x = 0; x < kProbeSize; ++x)
            if (image.pixelColor(x, y).alpha() > 60)
                ++covered;
    return covered;
}

} // namespace

QByteArray injectDash(const QByteArray &svg, qreal dashArray, qreal dashOffset) {
    const int rootEnd = svg.indexOf('>');
    if (rootEnd < 0)
        return svg;

    QByteArray attrs = " stroke-dasharray=\"";
    attrs += QByteArray::number(dashArray, 'f', 3);
    attrs += "\" stroke-dashoffset=\"";
    attrs += QByteArray::number(dashOffset, 'f', 3);
    attrs += '"';

    return svg.left(rootEnd) + attrs + svg.mid(rootEnd);
}

qreal measureStrokeLength(const QByteArray &svg) {
    // An unparseable document renders nothing; bisecting it would converge on
    // the cap and silently report a huge "length".
    if (strokeCoverage(svg) <= 0)
        return 0.0;

    // A dash of L with an offset of L puts the gap over the whole stroke, so the
    // smallest L that renders nothing is the longest subpath's length.
    if (strokeCoverage(injectDash(svg, kMaxLength, kMaxLength)) > 0)
        return 0.0;   // still visible at the cap: it's a fill, not a stroke

    qreal lo = 1.0, hi = kMaxLength;
    for (int i = 0; i < kBisectSteps; ++i) {
        const qreal mid = (lo + hi) / 2.0;
        if (strokeCoverage(injectDash(svg, mid, mid)) == 0)
            hi = mid;
        else
            lo = mid;
    }
    return hi;
}

} // namespace SvgStroke
