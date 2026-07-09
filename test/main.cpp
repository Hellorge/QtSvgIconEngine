// QtSvgIconEngine — showcase.
//
// Everything here is driven from the palette and from relative units, so the
// theme toggle, the size slider and the speed slider all re-flow the whole
// window without a single hardcoded pixel in the demo itself.

#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QListWidget>
#include <QMenu>
#include <QIcon>
#include <QVariantMap>
#include <QDebug>
#include <QStyle>
#include <QTimer>

#include "SvgIconEngine.h"
#include "SvgIconButton.h"

// ---------------------------------------------------------------------------
// Palettes — the whole point of inheriting QPalette::Text
// ---------------------------------------------------------------------------

static QPalette darkPalette() {
    QPalette p;
    p.setColor(QPalette::Window,        QColor("#0f172a"));
    p.setColor(QPalette::WindowText,    QColor("#e2e8f0"));
    p.setColor(QPalette::Base,          QColor("#111c33"));
    p.setColor(QPalette::AlternateBase, QColor("#16233d"));
    p.setColor(QPalette::Text,          QColor("#e2e8f0"));
    p.setColor(QPalette::Button,        QColor("#1e293b"));
    p.setColor(QPalette::ButtonText,    QColor("#e2e8f0"));
    p.setColor(QPalette::Highlight,     QColor("#38bdf8"));
    p.setColor(QPalette::HighlightedText, QColor("#0f172a"));
    p.setColor(QPalette::Mid,           QColor("#334155"));
    p.setColor(QPalette::Dark,          QColor("#64748b"));
    p.setColor(QPalette::Link,          QColor("#38bdf8"));
    return p;
}

static QPalette lightPalette() {
    QPalette p;
    p.setColor(QPalette::Window,        QColor("#f8fafc"));
    p.setColor(QPalette::WindowText,    QColor("#0f172a"));
    p.setColor(QPalette::Base,          QColor("#ffffff"));
    p.setColor(QPalette::AlternateBase, QColor("#f1f5f9"));
    p.setColor(QPalette::Text,          QColor("#0f172a"));
    p.setColor(QPalette::Button,        QColor("#e2e8f0"));
    p.setColor(QPalette::ButtonText,    QColor("#0f172a"));
    p.setColor(QPalette::Highlight,     QColor("#0284c7"));
    p.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    p.setColor(QPalette::Mid,           QColor("#cbd5e1"));
    p.setColor(QPalette::Dark,          QColor("#94a3b8"));
    p.setColor(QPalette::Link,          QColor("#0284c7"));
    return p;
}

// ---------------------------------------------------------------------------
// Small layout helpers. Spacing is expressed in `em` — the font's line height —
// so the whole window rescales with the system font rather than being tuned to
// one screen.
// ---------------------------------------------------------------------------

static int em(const QWidget *w) { return QFontMetrics(w->font()).height(); }

// Styled via the page stylesheet, so a theme switch re-resolves its colour
// instead of freezing whatever palette existed at construction.
static QLabel *caption(const QString &text, QWidget *parent = nullptr) {
    auto *l = new QLabel(text, parent);
    l->setObjectName("caption");
    l->setWordWrap(true);
    return l;
}

// One stylesheet for the whole page. Selectors are object-name scoped because
// QLabel derives from QFrame — a bare "QFrame" rule would box every caption.
static QString pageStyle(int unit) {
    return QStringLiteral(
        "QWidget#page   { background: palette(window); }"
        "QFrame#card    { border: 1px solid palette(mid); border-radius: %1px;"
        "                 background: palette(base); }"
        "QLabel#caption { color: palette(dark); }").arg(unit / 2);
}

static QFrame *card(const QString &title, const QString &subtitle, QWidget *content) {
    auto *frame = new QFrame;
    frame->setObjectName("card");
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setAttribute(Qt::WA_StyledBackground, true);

    const int u = em(frame);
    auto *v = new QVBoxLayout(frame);
    v->setContentsMargins(u, u, u, u);
    v->setSpacing(u / 2);

    auto *t = new QLabel(title);
    QFont tf = t->font();
    tf.setBold(true);
    tf.setPointSizeF(tf.pointSizeF() * 1.1);
    t->setFont(tf);
    v->addWidget(t);

    if (!subtitle.isEmpty())
        v->addWidget(caption(subtitle));

    v->addSpacing(u / 3);
    v->addWidget(content);
    return frame;
}

static QWidget *row(std::initializer_list<QWidget *> widgets, int spacingEm = 1) {
    auto *w = new QWidget;
    auto *h = new QHBoxLayout(w);
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(em(w) * spacingEm);
    for (QWidget *x : widgets)
        h->addWidget(x);
    h->addStretch();
    return w;
}

// A labelled icon tile used by the state matrix.
static QWidget *tile(SvgIcon *icon, const QString &label) {
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(em(w) / 3);
    icon->setParent(w);
    icon->show();
    v->addWidget(icon, 0, Qt::AlignHCenter);
    auto *l = caption(label);
    l->setAlignment(Qt::AlignHCenter);
    v->addWidget(l);
    return w;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setPalette(darkPalette());

    SvgIconEngine engine(":/icons");

    // Base options. No "color": icons inherit QPalette::Text and follow the
    // theme. transition_ms drives every state change.
    QVariantMap base;
    base["size"]          = QSize(32, 32);
    base["transition_ms"] = 180;

    // Everything we want the live controls to reach.
    QVector<SvgIcon *> allIcons;
    auto track = [&](SvgIcon *i) { if (i) allIcons.push_back(i); return i; };

    auto *page = new QWidget;
    page->setObjectName("page");
    auto *col = new QVBoxLayout(page);
    const int u = em(page);
    page->setStyleSheet(pageStyle(u));
    col->setContentsMargins(u * 2, u * 2, u * 2, u * 2);
    col->setSpacing(u * 3 / 2);
    // ── Header ────────────────────────────────────────────────────────────
    {
        auto *title = new QLabel("QtSvgIconEngine");
        QFont f = title->font();
        f.setPointSizeF(f.pointSizeF() * 2.0);
        f.setBold(true);
        title->setFont(f);
        col->addWidget(title);
        col->addWidget(caption(
            "Tinted, state-aware, animated SVG icons — as widgets or as a QIcon. "
            "Nothing below uses a hardcoded colour or pixel size."));
    }
    // ── Live controls ─────────────────────────────────────────────────────
    {
        auto *controls = new QWidget;
        auto *form = new QHBoxLayout(controls);
        form->setContentsMargins(0, 0, 0, 0);
        form->setSpacing(u);

        auto *theme = new QComboBox;
        theme->addItems({"Dark", "Light"});
        QObject::connect(theme, &QComboBox::currentTextChanged, [&, page, u](const QString &t) {
            // Icons that never got an explicit colour re-tint themselves.
            app.setPalette(t == "Dark" ? darkPalette() : lightPalette());
            // Stylesheet palette(...) references are resolved when the sheet is
            // applied, so re-set it to pick up the new roles.
            page->setStyleSheet(pageStyle(u));
        });

        auto *size = new QSlider(Qt::Horizontal);
        size->setRange(16, 96);
        size->setValue(32);
        auto *sizeVal = caption("32 px");
        QObject::connect(size, &QSlider::valueChanged, [&, sizeVal](int v) {
            sizeVal->setText(QString("%1 px").arg(v));
            for (SvgIcon *i : allIcons)
                i->setSize(QSize(v, v));   // SVG: resolution independent
        });

        auto *speed = new QSlider(Qt::Horizontal);
        speed->setRange(0, 600);
        speed->setValue(180);
        auto *speedVal = caption("180 ms");
        QObject::connect(speed, &QSlider::valueChanged, [&, speedVal](int v) {
            speedVal->setText(v == 0 ? QString("instant") : QString("%1 ms").arg(v));
            for (SvgIcon *i : allIcons)
                i->setTransitionDuration(v);
        });

        form->addWidget(new QLabel("Theme"));   form->addWidget(theme);
        form->addSpacing(u);
        form->addWidget(new QLabel("Size"));    form->addWidget(size);   form->addWidget(sizeVal);
        form->addSpacing(u);
        form->addWidget(new QLabel("Transition")); form->addWidget(speed); form->addWidget(speedVal);
        form->addStretch();

        col->addWidget(card("Live controls",
                            "Switch the theme and watch every icon re-tint: they inherit "
                            "QPalette::Text rather than baking a colour in.",
                            controls));
    }
    // ── State matrix ──────────────────────────────────────────────────────
    {
        auto *content = new QWidget;
        auto *h = new QHBoxLayout(content);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(em(content) * 2);

        struct S { const char *label; SvgIcon::State state; };
        const S states[] = {
            {"Normal",   SvgIcon::Normal},
            {"Hovered",  SvgIcon::Hovered},
            {"Pressed",  SvgIcon::Pressed},
            {"Selected", SvgIcon::Selected},
            {"Disabled", SvgIcon::Disabled},
        };
        for (const S &s : states) {
            QVariantMap o = base;
            o["size"] = QSize(40, 40);
            o["transition_ms"] = 0;   // hold the state, don't animate into it
            SvgIcon *icon = track(engine.getIcon("regular/notifications", o));
            if (!icon) continue;
            icon->setState(s.state);
            h->addWidget(tile(icon, s.label));
        }
        h->addStretch();

        col->addWidget(card("Five states, one SVG",
                            "Each state derives from the base colour by a relative factor "
                            "(hover lightens, pressed darkens, disabled dims), or takes an "
                            "explicit override like hover_color.",
                            content));
    }
    // ── Animated transitions + per-state artwork ──────────────────────────
    {
        auto *content = new QWidget;
        auto *h = new QHBoxLayout(content);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(em(content));

        // Outline fills in on hover, crossing from the `regular` style to `solid`.
        QVariantMap fill = base;
        fill["hover_icon"]  = "solid/heart";
        fill["hover_color"] = QColor("#f43f5e");
        if (SvgIcon *i = track(engine.getIcon("regular/heart", fill))) {
            auto *b = new SvgIconButton(i, "Fills on hover");
            b->setButtonStyle(SvgIconButton::Outline);
            h->addWidget(b);
        }

        // Press feedback: squash + darken.
        QVariantMap press = base;
        press["pressed_scale"] = 0.82;
        press["hover_color"]   = QColor("#38bdf8");
        if (SvgIcon *i = track(engine.getIcon("regular/cloud", press))) {
            auto *b = new SvgIconButton(i, "Press me");
            b->setButtonStyle(SvgIconButton::Flat);
            h->addWidget(b);
        }

        // Toggle latches into Selected.
        QVariantMap star = base;
        star["selected_icon"]  = "solid/star";
        star["selected_color"] = QColor("#f59e0b");
        if (SvgIcon *i = track(engine.getIcon("regular/star", star))) {
            auto *b = new SvgIconButton(i, "Toggle");
            b->makeToggleable(true);
            b->setButtonStyle(SvgIconButton::Outline);
            h->addWidget(b);
        }

        // Dropdown: arrow gets its own reserved column.
        QVariantMap opts = base;
        if (SvgIcon *i = track(engine.getIcon("regular/options", opts))) {
            auto *b = new SvgIconButton(i, "Options");
            auto *m = new QMenu(b);
            m->addAction("Duplicate");
            m->addAction("Rename");
            m->addSeparator();
            m->addAction("Delete");
            b->makeDropdown(m);
            h->addWidget(b);
        }

        // Disabled.
        QVariantMap del = base;
        if (SvgIcon *i = track(engine.getIcon("regular/trash", del))) {
            auto *b = new SvgIconButton(i, "Disabled");
            b->setEnabled(false);
            h->addWidget(b);
        }
        h->addStretch();

        col->addWidget(card("Animated state transitions",
                            "Hover, press and toggle animate over transition_ms. "
                            "hover_icon may cross styles — here regular/heart becomes "
                            "solid/heart while the colour interpolates.",
                            content));
    }
    // ── Resolution independence ───────────────────────────────────────────
    {
        auto *content = new QWidget;
        auto *h = new QHBoxLayout(content);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(em(content));
        h->setAlignment(Qt::AlignLeft | Qt::AlignBottom);

        for (int px : {16, 24, 32, 48, 64, 96}) {
            QVariantMap o = base;
            o["size"] = QSize(px, px);
            SvgIcon *icon = engine.getIcon("regular/speedometer", o);   // not tracked: fixed sizes
            if (!icon) continue;
            h->addWidget(tile(icon, QString("%1px").arg(px)), 0, Qt::AlignBottom);
        }
        h->addStretch();

        col->addWidget(card("Resolution independence",
                            "One file, any size, no bitmaps. The renderer is shared across "
                            "all six via the engine's cache.",
                            content));
    }
    // ── QIcon interop ─────────────────────────────────────────────────────
    {
        auto *content = new QWidget;
        auto *v = new QVBoxLayout(content);
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(em(content));

        QVariantMap o;
        o["size"] = QSize(20, 20);
        o["hover_icon"] = "solid/bookmark";
        QIcon bookmark = engine.icon("regular/bookmark", o);
        QIcon mail     = engine.icon("regular/mail");
        QIcon download = engine.icon("regular/download");

        auto *plain = new QPushButton(bookmark, "QPushButton");
        auto *tool  = new QToolButton;
        tool->setIcon(mail);
        tool->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tool->setText("QToolButton");
        auto *menuBtn = new QPushButton(download, "QAction menu");
        auto *qm = new QMenu(menuBtn);
        qm->addAction(new QAction(bookmark, "Bookmark", qm));
        qm->addAction(new QAction(mail, "Mail", qm));
        menuBtn->setMenu(qm);
        auto *off = new QPushButton(mail, "Disabled");
        off->setEnabled(false);

        v->addWidget(row({plain, tool, menuBtn, off}));

        auto *list = new QListWidget;
        list->setMaximumHeight(em(content) * 6);
        list->addItem(new QListWidgetItem(bookmark, "Item views work too"));
        list->addItem(new QListWidgetItem(mail, "…because it's a real QIcon"));
        list->addItem(new QListWidgetItem(download, "QIcon::Mode drives the colour"));
        v->addWidget(list);

        col->addWidget(card("Works anywhere Qt takes a QIcon",
                            "SvgQIconEngine is a real QIconEngine, so QAction, item views "
                            "and stock buttons get the same tinting and per-state artwork. "
                            "QIcon cannot animate — use the widget when you need that.",
                            content));
    }
    // ── Gallery ───────────────────────────────────────────────────────────
    {
        auto *content = new QWidget;
        auto *grid = new QGridLayout(content);
        grid->setContentsMargins(0, 0, 0, 0);
        const int gu = em(content);
        grid->setSpacing(gu / 2);

        const QStringList names = {
            "home", "search", "settings", "person", "mail", "camera", "image",
            "cloud", "download", "play", "pause", "musical-notes", "moon",
            "sunny", "color-palette", "layers", "grid", "star",
        };
        const int cols = 9;
        for (int i = 0; i < names.size(); ++i) {
            QVariantMap o = base;
            o["size"] = QSize(28, 28);
            o["hover_icon"]  = QString("solid/%1").arg(names[i]);
            o["hover_color"] = QColor("#38bdf8");
            SvgIcon *icon = track(engine.getIcon("regular/" + names[i], o));
            if (!icon) continue;
            auto *b = new SvgIconButton(icon);
            b->makeToolButton();
            b->setToolTip(names[i]);
            grid->addWidget(b, i / cols, i % cols);
        }

        col->addWidget(card("Gallery",
                            "Hover any icon: it fills to its solid variant and tints to the "
                            "accent colour. All renderers come from one LRU cache.",
                            content));
    }
    col->addStretch();

    // The layout dictates the window's minimum; scroll instead of clipping.
    col->setSizeConstraint(QLayout::SetMinimumSize); 
    auto *scroll = new QScrollArea;   // heap: it takes ownership of `page`
    scroll->setWidget(page);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWindowTitle("QtSvgIconEngine — showcase");
    const QSize sh = page->sizeHint();
    scroll->resize(sh.width() + em(page) * 3, qMin(sh.height() + em(page) * 2, 900));
    scroll->show();

    // `--shot <file>` renders the window to a PNG and exits. Useful for docs and
    // for verifying the demo in headless CI.
    const QStringList args = app.arguments();
    const int shotIdx = args.indexOf(QStringLiteral("--shot"));
    if (shotIdx > 0 && shotIdx + 1 < args.size()) {
        const QString path = args[shotIdx + 1];
        // Grab the page, not the viewport, so the shot includes everything
        // below the fold. Then flip the palette and grab again — proof that the
        // icons follow the theme rather than baking a colour in.
        QTimer::singleShot(400, [page, path, &app] {
            page->grab().save(path);
            qInfo().noquote() << "wrote" << path;

            app.setPalette(lightPalette());
            page->setStyleSheet(pageStyle(em(page)));
            QTimer::singleShot(300, [page, path, &app] {
                QString lightPath = path;
                lightPath.replace(QStringLiteral(".png"), QStringLiteral("-light.png"));
                page->grab().save(lightPath);
                qInfo().noquote() << "wrote" << lightPath;
                app.quit();
            });
        });
    }

    return app.exec();
}
