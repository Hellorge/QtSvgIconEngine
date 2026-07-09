// Renders the animation frames used by README.md, so the documentation is
// reproducible instead of hand-captured.
//
//   make_docs <svg-root> <out-dir>
//
// Then assemble with ffmpeg (see tools/make_docs.sh).

#include <QApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QtMath>
#include <QDebug>

#include "SvgIconEngine.h"
#include "SvgIcon.h"
#include "SvgIconButton.h"

static const int kFps = 50;
static const int kFrameMs = 1000 / kFps;

// ---------------------------------------------------------------------------
// Synthesized pointer. The clips are driven by real mouse events rather than by
// calling setState() directly, so what you see is the whole chain — enterEvent,
// syncIconState, the animation, and SvgIconButton repainting the hidden icon.
// ---------------------------------------------------------------------------

struct Pointer {
    QWidget *host = nullptr;
    QWidget *target = nullptr;
    QPointF pos;
    bool inside = false;
    bool down = false;

    void moveTo(const QPointF &p) {
        pos = p;
        const QPoint local = target->mapFrom(host, pos.toPoint());
        const bool nowInside = target->rect().contains(local);

        if (nowInside && !inside) {
            target->setAttribute(Qt::WA_UnderMouse, true);
            QEnterEvent e(local, local, pos);
            QApplication::sendEvent(target, &e);
            inside = true;
        } else if (!nowInside && inside) {
            target->setAttribute(Qt::WA_UnderMouse, false);
            QEvent e(QEvent::Leave);
            QApplication::sendEvent(target, &e);
            inside = false;
        }
        if (inside) {
            QMouseEvent e(QEvent::MouseMove, local, pos, Qt::NoButton,
                          down ? Qt::LeftButton : Qt::NoButton, Qt::NoModifier);
            QApplication::sendEvent(target, &e);
        }
    }

    void press() {
        down = true;
        const QPoint local = target->mapFrom(host, pos.toPoint());
        QMouseEvent e(QEvent::MouseButtonPress, local, pos, Qt::LeftButton,
                      Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(target, &e);
    }

    void release() {
        down = false;
        const QPoint local = target->mapFrom(host, pos.toPoint());
        QMouseEvent e(QEvent::MouseButtonRelease, local, pos, Qt::LeftButton,
                      Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(target, &e);
    }
};

// A classic arrow, drawn into the frame after the grab.
static void drawCursor(QImage &frame, const QPointF &p, bool down) {
    static const QPointF arrow[] = {
        {0,0}, {0,16}, {4,12.5}, {7,19}, {10,17.5}, {7,11.5}, {12,11}
    };
    QPainterPath path;
    path.moveTo(arrow[0]);
    for (int i = 1; i < 7; ++i) path.lineTo(arrow[i]);
    path.closeSubpath();

    QPainter g(&frame);
    g.setRenderHint(QPainter::Antialiasing);
    g.translate(p);
    if (down) {
        // a small click ring, so a press reads as a press
        g.setPen(QPen(QColor(255, 255, 255, 200), 1.5));
        g.setBrush(Qt::NoBrush);
        g.drawEllipse(QPointF(1, 1), 11, 11);
    }
    g.setPen(QPen(QColor(20, 20, 20, 220), 1.4));
    g.setBrush(QColor(250, 250, 250));
    g.drawPath(path);
}

static QPalette themed(bool dark) {
    QPalette p;
    p.setColor(QPalette::Window,    dark ? QColor("#0f172a") : QColor("#f8fafc"));
    p.setColor(QPalette::Base,      dark ? QColor("#0f172a") : QColor("#f8fafc"));
    p.setColor(QPalette::Text,      dark ? QColor("#e2e8f0") : QColor("#0f172a"));
    p.setColor(QPalette::WindowText,dark ? QColor("#e2e8f0") : QColor("#0f172a"));
    p.setColor(QPalette::Highlight, dark ? QColor("#38bdf8") : QColor("#0284c7"));
    p.setColor(QPalette::Mid,       dark ? QColor("#334155") : QColor("#cbd5e1"));
    p.setColor(QPalette::Dark,      dark ? QColor("#64748b") : QColor("#94a3b8"));
    p.setColor(QPalette::Button,    dark ? QColor("#1e293b") : QColor("#e2e8f0"));
    p.setColor(QPalette::ButtonText,dark ? QColor("#e2e8f0") : QColor("#0f172a"));
    return p;
}

// Pump the event loop for one frame's worth of wall-clock time, so
// QPropertyAnimation actually advances between grabs.
static void pumpFrame() {
    QElapsedTimer t; t.start();
    while (t.elapsed() < kFrameMs)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 2);
}

static void grab(QWidget *w, const QString &dir, int &n,
                 const Pointer *ptr = nullptr) {
    QImage frame = w->grab().toImage();
    if (ptr)
        drawCursor(frame, ptr->pos, ptr->down);
    frame.save(QString("%1/f%2.png").arg(dir).arg(n++, 4, 10, QChar('0')));
}

// Glide the pointer from a to b over `steps` frames, grabbing each one.
static void glide(Pointer &ptr, QPointF a, QPointF b, int steps,
                  QWidget *host, const QString &dir, int &n, void (*pump)()) {
    for (int i = 1; i <= steps; ++i) {
        const qreal t = qreal(i) / steps;
        const qreal e = t < 0.5 ? 2*t*t : 1 - qPow(-2*t + 2, 2) / 2;   // ease in-out
        ptr.moveTo(a + (b - a) * e);
        pump();
        grab(host, dir, n, &ptr);
    }
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc < 3) { qWarning() << "usage: make_docs <svg-root> <out-dir>"; return 2; }
    const QString root = argv[1], out = argv[2];
    app.setPalette(themed(true));

    SvgIconEngine engine(root);

    // ── hover: outline fills to solid, colour interpolates ─────────────────
    // Driven by a synthesized pointer, so this exercises enterEvent ->
    // syncIconState -> animateTo -> the button repainting the hidden icon.
    {
        const QString dir = out + "/hover"; QDir().mkpath(dir); int n = 0;
        QVariantMap o;
        o["size"] = QSize(48, 48);
        o["transition_ms"] = 380;
        o["hover_icon"]  = "solid/heart";
        o["hover_color"] = QColor("#f43f5e");
        SvgIcon *icon = engine.getIcon("regular/heart", o);

        QWidget host; host.setAutoFillBackground(true); host.resize(300, 130);
        auto *btn = new SvgIconButton(icon, "Like", &host);
        btn->setButtonStyle(SvgIconButton::Outline);
        btn->move((host.width() - btn->sizeHint().width()) / 2,
                  (host.height() - btn->sizeHint().height()) / 2);
        btn->resize(btn->sizeHint());
        host.show(); app.processEvents();

        Pointer ptr{&host, btn, QPointF(20, 118), false, false};
        const QPointF over = QPointF(btn->geometry().center()) + QPointF(-6, 4);

        for (int i = 0; i < 6; ++i)  { pumpFrame(); grab(&host, dir, n, &ptr); }
        glide(ptr, ptr.pos, over, 14, &host, dir, n, pumpFrame);
        if (icon->state() != SvgIcon::Hovered)
            qWarning() << "hover: pointer did not reach the icon (state" << int(icon->state()) << ")";
        for (int i = 0; i < 22; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }
        glide(ptr, over, QPointF(280, 118), 14, &host, dir, n, pumpFrame);
        for (int i = 0; i < 20; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }
        qInfo() << "hover frames:" << n << " end state:" << int(icon->state());
    }

    // ── press: squash + darken, via real mouse press/release ───────────────
    {
        const QString dir = out + "/press"; QDir().mkpath(dir); int n = 0;
        QVariantMap o;
        o["size"] = QSize(48, 48);
        o["transition_ms"] = 220;
        o["pressed_scale"] = 0.72;
        o["pressed_color"] = QColor("#f97316");
        o["hover_color"]   = QColor("#38bdf8");
        SvgIcon *icon = engine.getIcon("regular/cloud", o);

        QWidget host; host.setAutoFillBackground(true); host.resize(300, 130);
        auto *btn = new SvgIconButton(icon, "Upload", &host);
        btn->setButtonStyle(SvgIconButton::Flat);
        btn->resize(btn->sizeHint());
        btn->move((host.width() - btn->width()) / 2, (host.height() - btn->height()) / 2);
        host.show(); app.processEvents();

        Pointer ptr{&host, btn, QPointF(20, 118), false, false};
        const QPointF over = QPointF(btn->geometry().center()) + QPointF(-6, 4);

        for (int i = 0; i < 4; ++i)  { pumpFrame(); grab(&host, dir, n, &ptr); }
        glide(ptr, ptr.pos, over, 12, &host, dir, n, pumpFrame);
        for (int i = 0; i < 10; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }

        ptr.press();
        if (icon->state() != SvgIcon::Pressed)
            qWarning() << "press: click did not reach the icon (state" << int(icon->state()) << ")";
        for (int i = 0; i < 16; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }

        ptr.release();
        for (int i = 0; i < 14; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }
        glide(ptr, over, QPointF(280, 118), 12, &host, dir, n, pumpFrame);
        for (int i = 0; i < 14; ++i) { pumpFrame(); grab(&host, dir, n, &ptr); }
        qInfo() << "press frames:" << n << " end state:" << int(icon->state());
    }

    // ── theme: icons with no explicit colour follow QPalette::Text ─────────
    {
        const QString dir = out + "/theme"; QDir().mkpath(dir); int n = 0;
        QWidget host; host.setAutoFillBackground(true);
        auto *l = new QHBoxLayout(&host);
        l->setSpacing(18);
        for (const char *name : {"home", "search", "settings", "mail", "star"}) {
            QVariantMap o; o["size"] = QSize(56, 56);   // no "color" key
            l->addWidget(engine.getIcon(QString("regular/") + name, o));
        }
        host.resize(430, 100); host.show(); app.processEvents();

        for (bool dark : {true, false, true, false}) {
            app.setPalette(themed(dark));
            app.processEvents();
            for (int i = 0; i < 18; ++i) { pumpFrame(); grab(&host, dir, n); }
        }
        qInfo() << "theme frames:" << n;
    }

    return 0;
}
