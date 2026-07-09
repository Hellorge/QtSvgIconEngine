#ifndef SVGSTROKE_H
#define SVGSTROKE_H

#include <QByteArray>

// Stroke effects, implemented by rewriting the SVG source rather than by
// parsing its geometry. `stroke-dasharray` and `stroke-dashoffset` inherit, so
// injecting them once on the root <svg> element reaches every child.
//
// Qt's SVG renderer honours both (verified in tests/tst_stroke.cpp).
namespace SvgStroke {

// Insert dash attributes on the root element. Existing per-element `style`
// rules that don't mention dashing are unaffected, so this composes with the
// icons' own stroke-width / linecap.
QByteArray injectDash(const QByteArray &svg, qreal dashArray, qreal dashOffset);

// The length of the longest subpath, in user units — i.e. the dash length that
// exactly hides the stroke. Found by bisection on rendered coverage, which
// avoids implementing an SVG path-data parser (arcs, beziers, the lot).
//
// Returns 0 for artwork with no stroke to draw (a filled icon), for which the
// caller should treat any stroke effect as a no-op.
qreal measureStrokeLength(const QByteArray &svg);

} // namespace SvgStroke

#endif // SVGSTROKE_H
