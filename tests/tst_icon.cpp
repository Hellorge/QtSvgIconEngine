#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

#include <QElapsedTimer>
#include "SvgIconEngine.h"
#include "SvgIcon.h"

static int failures = 0;
static void check(const char *name, bool ok) {
    qInfo().noquote() << (ok ? "  PASS  " : "  FAIL  ") << name;
    if (!ok) failures++;
}
static void pump(int ms) {  // let animations run
    QElapsedTimer t; t.start();
    while (t.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 5);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    const QString root = argv[1];
    QDir d(root + "/regular");
    QStringList names = d.entryList(QStringList() << "*.svg", QDir::Files);
    names.sort();
    auto nm = [&](int i){ return QFileInfo(names[i]).completeBaseName(); };
    QVariantMap base; base["size"] = QSize(64,64);

    qInfo().noquote() << "--- lifetime (regression) ---";
    {
        SvgIconEngine e(root);
        e.setCacheLimit(100);
        QVariantMap o = base;
        SvgIcon *held = e.getIcon("regular/" + nm(0), o);
        bool gone = false;
        QObject::connect(held->renderer(), &QObject::destroyed, [&]{ gone = true; });
        for (int i = 1; i < 141; ++i) { QVariantMap oo = base; delete e.getIcon("regular/" + nm(i), oo); }
        check("cache eviction does not dangle a live icon's renderer", !gone);
        delete held;
    }
    {
        SvgIconEngine e(root);
        e.setCacheLimit(0);
        QVariantMap o = base;
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        bool gone = false;
        QObject::connect(icon->renderer(), &QObject::destroyed, [&]{ gone = true; });
        delete icon;
        check("cache limit 0 frees renderer with the icon (no leak)", gone);
    }

    qInfo().noquote() << "--- palette inheritance ---";
    {
        SvgIconEngine e(root);
        QVariantMap o = base;                    // no explicit colour
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        check("no explicit colour -> icon uses palette text colour",
              icon->color() == QApplication::palette().text().color());

        QPalette p = QApplication::palette();
        p.setColor(QPalette::Text, QColor("#38bdf8"));
        QApplication::setPalette(p);             // simulate a theme switch
        pump(50);
        check("theme change re-tints an inherited colour", icon->color() == QColor("#38bdf8"));
        delete icon;
    }
    {
        QVariantMap o = base; o["color"] = QColor(Qt::red);
        SvgIconEngine e(root);
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        QPalette p = QApplication::palette();
        p.setColor(QPalette::Text, QColor(Qt::green));
        QApplication::setPalette(p);
        pump(50);
        check("theme change does NOT override an explicit colour", icon->color() == QColor(Qt::red));
        delete icon;
    }

    qInfo().noquote() << "--- per-state options ---";
    {
        SvgIconEngine e(root);
        QVariantMap o = base;
        o["color"]          = QColor("#808080");
        o["hover_color"]    = QColor("#ff0000");
        o["pressed_color"]  = QColor("#00ff00");
        o["transition_ms"]  = 0;                 // instant, so we can assert directly
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);

        check("Normal uses base colour",  icon->color() == QColor("#808080"));
        icon->setState(SvgIcon::Hovered);
        check("Hovered uses hover_color", icon->color() == QColor("#ff0000"));
        icon->setState(SvgIcon::Pressed);
        check("Pressed uses pressed_color", icon->color() == QColor("#00ff00"));
        icon->setState(SvgIcon::Normal);
        check("returning to Normal restores base colour", icon->color() == QColor("#808080"));
        delete icon;
    }
    {
        SvgIconEngine e(root);
        QVariantMap o = base;
        o["color"] = QColor("#808080");
        o["transition_ms"] = 0;
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        icon->setState(SvgIcon::Hovered);
        check("Hovered falls back to a lighter base when unset",
              icon->color() == QColor("#808080").lighter(130));
        icon->setState(SvgIcon::Disabled);
        check("Disabled dims opacity by default", qFuzzyCompare(icon->opacity(), 0.5));
        delete icon;
    }

    qInfo().noquote() << "--- animated transitions (the main feature) ---";
    {
        SvgIconEngine e(root);
        QVariantMap o = base;
        o["color"] = QColor("#000000");
        o["hover_color"] = QColor("#ffffff");
        o["transition_ms"] = 200;
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        int repaints = 0;
        QObject::connect(icon, &SvgIcon::visualChanged, [&]{ ++repaints; });

        icon->setState(SvgIcon::Hovered);
        pump(60);
        const QColor mid = icon->color();
        const bool interpolating = mid != QColor("#000000") && mid != QColor("#ffffff");
        check("colour is mid-transition shortly after setState (it animates)", interpolating);

        pump(300);
        check("transition settles on the target colour", icon->color() == QColor("#ffffff"));
        // one emit per animation frame -> a host repainting on this signal sees
        // a smooth transition, not a jump
        check("visualChanged() fires many times across the transition", repaints > 3);
        qInfo().noquote() << QString("         (observed %1 repaint signals over a 200ms transition)").arg(repaints);
        delete icon;
    }
    {
        SvgIconEngine e(root);
        QVariantMap o = base; o["transition_ms"] = 0;
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        QColor before = icon->color();
        icon->setState(SvgIcon::Hovered);
        check("transition_ms=0 restores instant switching", icon->color() != before);
        delete icon;
    }

    qInfo().noquote() << "--- per-state artwork ---";
    {
        SvgIconEngine e(root);
        QVariantMap o = base;
        o["hover_icon"] = nm(5);                 // a different SVG for hover
        o["transition_ms"] = 0;
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        QSvgRenderer *normal = icon->renderer();
        check("state icon registered", icon->hasStateRenderer(SvgIcon::Hovered));
        icon->setState(SvgIcon::Hovered);
        check("hover swaps to its own artwork", icon->renderer() != normal);
        icon->setState(SvgIcon::Normal);
        check("leaving hover restores base artwork", icon->renderer() == normal);
        delete icon;
    }
    {
        SvgIconEngine e(root);
        QVariantMap o = base;
        o["hover_icon"] = "does_not_exist";
        SvgIcon *icon = e.getIcon("regular/" + nm(0), o);
        check("missing state icon degrades gracefully (icon still created)", icon != nullptr);
        check("missing state icon leaves no override", icon && !icon->hasStateRenderer(SvgIcon::Hovered));
        delete icon;
    }

    qInfo().noquote() << (failures ? QString("%1 TEST(S) FAILED").arg(failures) : QString("ALL TESTS PASSED"));
    return failures;
}
