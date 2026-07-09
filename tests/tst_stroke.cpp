// Stroke effects: draw-on via stroke-dashoffset, and marching-ants dashing.
//
// These work by rewriting the SVG source rather than parsing its geometry, so
// the tests pin down both halves: that Qt honours the injected properties, and
// that path length can be recovered by bisection instead of a path parser.

#include <QApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPropertyAnimation>
#include <QElapsedTimer>
#include <QSvgRenderer>
#include <QDebug>

#include "SvgIconEngine.h"
#include "SvgIcon.h"
#include "SvgStroke.h"

static int fails = 0;
static void check(const char *n, bool ok) {
    qInfo().noquote() << (ok ? "  PASS  " : "  FAIL  ") << n;
    if (!ok) ++fails;
}
static void pump(int ms) {
    QElapsedTimer t; t.start();
    while (t.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 2);
}
static QImage snap(SvgIcon *icon) {
    QImage im(icon->size(), QImage::Format_ARGB32_Premultiplied);
    im.fill(Qt::transparent);
    { QPainter p(&im); icon->render(&p, QPoint(), QRegion(), QWidget::DrawChildren); }
    return im;
}
static int coverage(SvgIcon *icon) {
    QImage im(icon->size(), QImage::Format_ARGB32_Premultiplied);
    im.fill(Qt::transparent);
    { QPainter p(&im); icon->render(&p, QPoint(), QRegion(), QWidget::DrawChildren); }
    int n = 0;
    for (int y = 0; y < im.height(); ++y)
        for (int x = 0; x < im.width(); ++x)
            if (im.pixelColor(x, y).alpha() > 60) ++n;
    return n;
}
static QByteArray read(const QString &p) { QFile f(p); f.open(QIODevice::ReadOnly); return f.readAll(); }

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    const QString root = argv[1];
    SvgIconEngine e(root);

    // ── the primitive: Qt honours what we inject ───────────────────────────
    {
        const QByteArray svg = read(root + "/regular/heart.svg");
        check("injectDash puts attributes on the root element",
              SvgStroke::injectDash(svg, 100, 50).contains("stroke-dasharray=\"100.000\""));

        const qreal len = SvgStroke::measureStrokeLength(svg);
        check("stroked icon reports a plausible path length", len > 100 && len < 16384);

        const QByteArray filled = read(root + "/solid/heart.svg");
        check("filled icon reports zero stroke length",
              qFuzzyIsNull(SvgStroke::measureStrokeLength(filled)));
    }

    // ── draw-on: coverage grows monotonically with progress ────────────────
    {
        QVariantMap o;
        o["size"] = QSize(64, 64);
        o["color"] = QColor("#ffffff");
        o["stroke_progress"] = 0.0;
        SvgIcon *icon = e.getIcon("regular/heart", o);

        check("engine measured a stroke length", icon->strokeLength() > 0);

        const int at0 = coverage(icon);
        icon->setStrokeProgress(0.25); const int at25 = coverage(icon);
        icon->setStrokeProgress(0.50); const int at50 = coverage(icon);
        icon->setStrokeProgress(1.00); const int at100 = coverage(icon);

        qInfo().noquote() << QString("         coverage 0%%=%1  25%%=%2  50%%=%3  100%%=%4")
            .arg(at0).arg(at25).arg(at50).arg(at100);

        check("progress 0 draws nothing", at0 == 0);
        check("coverage increases with progress", at0 < at25 && at25 < at50 && at50 < at100);
        check("progress 1 draws the whole icon", at100 > 0);

        icon->setStrokeProgress(1.0);
        const int full = coverage(icon);
        QVariantMap plain; plain["size"] = QSize(64,64); plain["color"] = QColor("#ffffff");
        SvgIcon *ref = e.getIcon("regular/heart", plain);
        check("fully drawn matches an icon with no effect at all",
              qAbs(full - coverage(ref)) <= 2);
        delete icon; delete ref;
    }

    // ── a filled icon ignores the effect rather than vanishing ─────────────
    {
        QVariantMap o;
        o["size"] = QSize(64, 64);
        o["color"] = QColor("#ffffff");
        o["stroke_progress"] = 0.0;
        SvgIcon *icon = e.getIcon("solid/heart", o);
        check("filled icon: stroke effect is a no-op, not a blank icon", coverage(icon) > 0);
        delete icon;
    }

    // ── it is a Q_PROPERTY, so it animates like any other ──────────────────
    {
        QVariantMap o;
        o["size"] = QSize(64, 64);
        o["color"] = QColor("#ffffff");
        o["stroke_progress"] = 0.0;
        SvgIcon *icon = e.getIcon("regular/star", o);

        int repaints = 0;
        QObject::connect(icon, &SvgIcon::visualChanged, [&]{ ++repaints; });

        auto *anim = new QPropertyAnimation(icon, "stroke_progress");
        anim->setDuration(200);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->start();
        pump(60);
        const qreal mid = icon->strokeProgress();
        pump(260);

        check("stroke_progress animates (mid-flight value strictly between)",
              mid > 0.0 && mid < 1.0);
        check("animation settles fully drawn", qFuzzyCompare(icon->strokeProgress(), 1.0));
        check("each frame repaints", repaints > 3);
        qInfo().noquote() << QString("         (%1 repaints, mid-flight progress %2)")
            .arg(repaints).arg(mid, 0, 'f', 2);
        delete icon;
    }

    // ── per-state: hover draws the icon on ─────────────────────────────────
    {
        QVariantMap o;
        o["size"] = QSize(64, 64);
        o["color"] = QColor("#ffffff");
        o["transition_ms"] = 0;
        o["stroke_progress"] = 0.35;          // partially drawn at rest
        o["hover_stroke_progress"] = 1.0;     // completes on hover
        SvgIcon *icon = e.getIcon("regular/settings", o);

        const int rest = coverage(icon);
        icon->setState(SvgIcon::Hovered);
        const int hovered = coverage(icon);
        icon->setState(SvgIcon::Normal);
        const int back = coverage(icon);

        check("hover_stroke_progress completes the drawing", hovered > rest);
        check("leaving hover returns to the resting progress", qAbs(back - rest) <= 2);
        delete icon;
    }

    // ── marching ants ──────────────────────────────────────────────────────
    {
        QVariantMap plain; plain["size"] = QSize(64,64); plain["color"] = QColor("#ffffff");
        SvgIcon *solidLine = e.getIcon("regular/heart", plain);
        const int undashed = coverage(solidLine);

        QVariantMap o;
        o["size"] = QSize(64, 64);
        o["color"] = QColor("#ffffff");
        o["dash_pattern"] = 40.0;
        SvgIcon *icon = e.getIcon("regular/heart", o);
        const QImage a = snap(icon);
        const int dashed = coverage(icon);

        icon->setDashOffset(40.0);      // shift by one full dash+gap period
        const QImage b = snap(icon);

        check("dash_pattern removes ink (gaps appear)", dashed > 0 && dashed < undashed);
        check("dash_offset moves the dashes (pixels differ)", a != b);
        qInfo().noquote() << QString("         undashed=%1px  dashed=%2px").arg(undashed).arg(dashed);
        delete icon; delete solidLine;
    }

    qInfo().noquote() << (fails ? QString("%1 FAILED").arg(fails)
                                : QString("ALL STROKE TESTS PASSED"));
    return fails;
}
