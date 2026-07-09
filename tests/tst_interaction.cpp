// Drives an SvgIconButton with synthesized mouse events and checks that the
// state actually reaches the icon. Every earlier bug in this chain — the button
// never telling the icon it was pressed, and the hidden icon's update() being a
// no-op so animation frames never painted — is invisible to a unit test that
// calls setState() directly.

#include <QApplication>
#include <QEnterEvent>
#include <QMouseEvent>
#include <QElapsedTimer>
#include <QDebug>

#include "SvgIconEngine.h"
#include "SvgIcon.h"
#include "SvgIconButton.h"

static int fails = 0;
static void check(const char *n, bool ok) {
    qInfo().noquote() << (ok ? "  PASS  " : "  FAIL  ") << n;
    if (!ok) ++fails;
}

static void pump(int ms) {
    QElapsedTimer t; t.start();
    while (t.elapsed() < ms)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 2);
}

static void enter(QWidget *w) {
    const QPointF c = QPointF(w->rect().center());
    w->setAttribute(Qt::WA_UnderMouse, true);
    QEnterEvent e(c, c, c);
    QApplication::sendEvent(w, &e);
}
static void leave(QWidget *w) {
    w->setAttribute(Qt::WA_UnderMouse, false);
    QEvent e(QEvent::Leave);
    QApplication::sendEvent(w, &e);
}
static void press(QWidget *w) {
    const QPointF c = QPointF(w->rect().center());
    QMouseEvent e(QEvent::MouseButtonPress, c, c, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(w, &e);
}
static void release(QWidget *w) {
    const QPointF c = QPointF(w->rect().center());
    QMouseEvent e(QEvent::MouseButtonRelease, c, c, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(w, &e);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    SvgIconEngine e(argv[1]);

    QVariantMap o;
    o["size"] = QSize(32, 32);
    o["transition_ms"] = 0;            // settle instantly, assert directly
    o["hover_icon"] = "solid/heart";
    SvgIcon *icon = e.getIcon("regular/heart", o);
    auto *btn = new SvgIconButton(icon, "Like");
    btn->resize(btn->sizeHint());
    btn->show();
    app.processEvents();
    leave(btn);                        // offscreen platform parks the cursor at 0,0

    check("starts Normal", icon->state() == SvgIcon::Normal);

    enter(btn);
    check("mouse enter -> icon Hovered", icon->state() == SvgIcon::Hovered);
    check("hover swaps to per-state artwork", icon->hasStateRenderer(SvgIcon::Hovered));

    press(btn);
    check("mouse press -> icon Pressed", icon->state() == SvgIcon::Pressed);

    release(btn);
    check("release while inside -> back to Hovered", icon->state() == SvgIcon::Hovered);

    leave(btn);
    check("mouse leave -> icon Normal", icon->state() == SvgIcon::Normal);

    btn->setEnabled(false);
    check("disabling -> icon Disabled", icon->state() == SvgIcon::Disabled);
    btn->setEnabled(true);
    check("re-enabling -> icon Normal", icon->state() == SvgIcon::Normal);

    // Disabled must win over hover.
    btn->setEnabled(false);
    enter(btn);
    check("disabled beats hovered", icon->state() == SvgIcon::Disabled);
    leave(btn);
    btn->setEnabled(true);

    // Toggling latches Selected.
    btn->setCheckable(true);
    btn->setChecked(true);
    check("checking -> icon Selected", icon->state() == SvgIcon::Selected);
    enter(btn);
    check("hover beats checked", icon->state() == SvgIcon::Hovered);
    leave(btn);
    check("leaving a checked button -> Selected", icon->state() == SvgIcon::Selected);
    btn->setChecked(false);

    // The hidden icon must ask its host to repaint, or animation is invisible.
    {
        QVariantMap ao;
        ao["size"] = QSize(32, 32);
        ao["transition_ms"] = 200;
        ao["hover_color"] = QColor("#38bdf8");
        SvgIcon *anim = e.getIcon("regular/star", ao);
        auto *b2 = new SvgIconButton(anim, "Star");
        b2->resize(b2->sizeHint());
        b2->show();
        app.processEvents();
        leave(b2);

        int repaints = 0;
        QObject::connect(anim, &SvgIcon::visualChanged, [&]{ ++repaints; });
        enter(b2);
        pump(260);
        check("hovering animates (many repaint signals)", repaints > 3);
        check("animation settles on the hover colour", anim->color() == QColor("#38bdf8"));
        qInfo().noquote() << QString("         (%1 repaint signals over a 200ms transition)").arg(repaints);
        delete b2;
    }

    delete btn;
    qInfo().noquote() << (fails ? QString("%1 FAILED").arg(fails)
                                : QString("ALL INTERACTION TESTS PASSED"));
    return fails;
}
