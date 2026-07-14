// Every code block from README.md, verbatim. A documented snippet that stops
// compiling, or stops behaving as documented, fails here.
#include <QApplication>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QDebug>
#include <QTemporaryDir>
#include <QFile>
#include "SvgIconEngine.h"
#include "SvgIconButton.h"
#include "SvgIcon.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    SvgIconEngine icons(argv[1]);

    // ── Quick start ──────────────────────────────────────────────────────
    QPushButton *save = new QPushButton(icons.icon("regular/save"), "Save");
    {
        QVariantMap opts;
        opts["size"]        = QSize(24, 24);
        opts["hover_icon"]  = "solid/heart";
        opts["hover_color"] = QColor("#f43f5e");
        auto *like = new SvgIconButton(icons.getIcon("regular/heart", opts), "Like");
        Q_UNUSED(like)
    }

    // ── Addressing ───────────────────────────────────────────────────────
    icons.icon("regular/heart");
    icons.icon("regular/heart.svg");

    // ── States ───────────────────────────────────────────────────────────
    {
        QVariantMap o;
        o["color"]         = QColor("#94a3b8");
        o["hover_color"]   = QColor("#38bdf8");
        o["pressed_scale"] = 0.85;
        o["selected_icon"] = "solid/star";
        o["transition_ms"] = 180;
        SvgIcon *icon = icons.getIcon("regular/star", o);
        icon->setState(SvgIcon::Hovered);
    }

    // ── Stroke effects: draw-on ──────────────────────────────────────────
    {
        QVariantMap o;
        o["size"]            = QSize(48, 48);
        o["stroke_progress"] = 0.0;
        SvgIcon *icon = icons.getIcon("regular/star", o);

        auto *draw = new QPropertyAnimation(icon, "stroke_progress", icon);
        draw->setDuration(800);
        draw->setStartValue(0.0);
        draw->setEndValue(1.0);
        draw->setEasingCurve(QEasingCurve::InOutCubic);
        draw->start();
    }

    // ── Stroke effects: per-state ────────────────────────────────────────
    {
        QVariantMap o;
        o["stroke_progress"]       = 0.35;
        o["hover_stroke_progress"] = 1.0;
        o["transition_ms"]         = 400;
        auto *btn = new SvgIconButton(icons.getIcon("regular/heart", o), "Like");
        Q_UNUSED(btn)
    }

    // ── Stroke effects: marching ants ────────────────────────────────────
    {
        QVariantMap o;
        o["size"]         = QSize(48, 48);
        o["dash_pattern"] = 36.0;
        SvgIcon *icon = icons.getIcon("regular/cloud", o);

        auto *ants = new QPropertyAnimation(icon, "dash_offset", icon);
        ants->setDuration(1400);
        ants->setStartValue(0.0);
        ants->setEndValue(72.0);
        ants->setLoopCount(-1);
        ants->start();
    }

    // ── Querying stroke length (README "Limitation") ─────────────────────
    const qreal stroked = icons.strokeLength("regular/star");
    const qreal filled  = icons.strokeLength("solid/heart");

    // ── Widget or QIcon ──────────────────────────────────────────────────
    QVariantMap opts; opts["size"] = QSize(24,24);
    SvgIcon *w = icons.getIcon("regular/heart", opts);
    QIcon    i = icons.icon("regular/heart", opts);

    // ── Sprites ──────────────────────────────────────────────────────────
    QTemporaryDir tmp;
    const QString sheet = tmp.path() + "/toolbar.svg";
    {
        QFile f(sheet);
        if (!f.open(QIODevice::WriteOnly))
            return 1;
        f.write("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 10'>"
                "<rect id='cut' x='0' y='0' width='10' height='10' fill='#000'/>"
                "<circle id='paste' cx='15' cy='5' r='5' fill='#000'/></svg>");
    }
    SvgIcon *sw = icons.getIconFromSprite(sheet, "cut");
    QIcon    si = icons.iconFromSprite(sheet, "paste");

    // ── Caching ──────────────────────────────────────────────────────────
    icons.setCacheLimit(200);
    icons.clearCache();

    const bool ok = !save->icon().isNull() && w && !i.isNull()
                 && stroked > 0.0 && qFuzzyIsNull(filled)
                 && sw != nullptr && !si.isNull();
    qInfo().noquote() << (ok ? "  PASS  every README snippet compiles and behaves as documented"
                             : "  FAIL  a README snippet is wrong");
    qInfo().noquote() << QString("         stroked len=%1  filled len=%2 (README: a filled icon reports 0)")
        .arg(stroked, 0, 'f', 0).arg(filled, 0, 'f', 0);
    return ok ? 0 : 1;
}
