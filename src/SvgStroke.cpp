// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgStroke.h"

#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include <cctype>

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

// Index of the '>' closing the root <svg ...> opening tag, or -1.
//
// Not svg.indexOf('>'): a document may open with an XML declaration, a doctype
// or a comment, whose '>' comes first. A '>' may also appear unescaped inside an
// attribute value, so quoted spans are skipped.
int svgTagEnd(const QByteArray &svg) {
    int start = -1;
    for (int i = svg.indexOf("<svg"); i >= 0; i = svg.indexOf("<svg", i + 4)) {
        const int after = i + 4;
        if (after >= svg.size())
            break;
        // Reject "<svgfoo": the name must end here.
        const char c = svg.at(after);
        if (c == '>' || c == '/' || std::isspace(static_cast<unsigned char>(c))) {
            start = i;
            break;
        }
    }
    if (start < 0)
        return -1;

    char quote = 0;
    for (int i = start + 4; i < svg.size(); ++i) {
        const char c = svg.at(i);
        if (quote) {
            if (c == quote)
                quote = 0;
        } else if (c == '"' || c == '\'') {
            quote = c;
        } else if (c == '>') {
            return i;
        }
    }
    return -1;
}

} // namespace

QByteArray injectDash(const QByteArray &svg, qreal dashArray, qreal dashOffset) {
    int at = svgTagEnd(svg);
    if (at < 0)
        return svg;

    // A self-closing root (<svg .../>) has nothing to stroke, but insert before
    // the slash rather than after it so the document stays well-formed.
    if (at > 0 && svg.at(at - 1) == '/')
        --at;

    QByteArray attrs = " stroke-dasharray=\"";
    attrs += QByteArray::number(dashArray, 'f', 3);
    attrs += "\" stroke-dashoffset=\"";
    attrs += QByteArray::number(dashOffset, 'f', 3);
    attrs += '"';

    return svg.left(at) + attrs + svg.mid(at);
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
