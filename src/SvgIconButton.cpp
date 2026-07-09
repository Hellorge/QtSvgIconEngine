// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIconButton.h"
#include <QPainter>
#include <QStyleOption>
#include <QStylePainter>
#include <QApplication>

SvgIconButton::SvgIconButton(SvgIcon* icon, QWidget* parent)
    : QPushButton(parent)
    , m_icon(icon)
    , m_style(ButtonStyle::Default)
    , m_menu(nullptr)
    , m_isPressed(false)
    , m_isHovered(false)
{
    init();
}

SvgIconButton::SvgIconButton(SvgIcon* icon, const QString& text, QWidget* parent)
    : QPushButton(text, parent)
    , m_icon(icon)
    , m_style(ButtonStyle::Default)
    , m_menu(nullptr)
    , m_isPressed(false)
    , m_isHovered(false)
{
    init();
}

SvgIconButton::~SvgIconButton() {
    // m_icon is parented to this widget via setParent(this) in init(),
    // so Qt's object tree will delete it automatically. Do not delete manually.
}

void SvgIconButton::init() {
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAttribute(Qt::WA_StyledBackground, true);

    // Checking the button drives the icon's Selected state.
    connect(this, &QAbstractButton::toggled, this, [this](bool) { syncIconState(); });

    if (m_icon) {
        // Transfer ownership to this widget so Qt manages lifetime
        m_icon->setParent(this);
        // Hide the icon widget — we render it manually in paintEvent. A hidden
        // widget receives no paint events, so its own update() is a no-op; we
        // must repaint ourselves on every animation frame.
        m_icon->hide();
        connect(m_icon, &SvgIcon::visualChanged, this, QOverload<>::of(&QWidget::update));
        // A resized icon changes our sizeHint; a recoloured one does not.
        connect(m_icon, &SvgIcon::sizeChanged, this, [this]{ updateGeometry(); update(); });
    }
    updateGeometry();
}

// ---------------------------------------------------------------------------
// Relative layout metrics — the button's own "em"-based unit system
// ---------------------------------------------------------------------------

int SvgIconButton::em() const {
    return QFontMetrics(font()).height();
}

int SvgIconButton::iconTextGap() const {
    return qMax(2, em() / 2);          // ~0.5em
}

int SvgIconButton::textPadding() const {
    return qMax(2, em() / 2);          // ~0.5em
}

int SvgIconButton::verticalPadding() const {
    return qMax(2, em() / 4);          // ~0.25em
}

int SvgIconButton::arrowWidth() const {
    QStyleOptionButton option;
    initStyleOption(&option);
    // The style knows how wide its own indicator is; guessing 10px was wrong on
    // any theme or DPI that disagreed.
    const int pm = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this);
    return pm > 0 ? pm : qMax(6, em() / 2);
}

int SvgIconButton::arrowGap() const {
    return qMax(2, em() / 3);          // ~0.33em
}

// Icon + gap + text + arrow. The style adds its own frame and padding on top,
// which is why this must not be turned into a setFixedSize() at construction:
// borders, stylesheet padding, fonts and menus all change after init().
QSize SvgIconButton::contentSize() const {
    const QSize iconSize = m_icon ? m_icon->size() : QSize(0, 0);
    const QFontMetrics fm(font());

    int w = iconSize.width();
    int h = qMax(iconSize.height(), fm.height()) + verticalPadding();

    if (!text().isEmpty()) {
        if (w > 0)
            w += iconTextGap();
        w += fm.horizontalAdvance(text()) + textPadding();
    }
    if (m_menu)
        w += arrowGap() + arrowWidth();

    return QSize(w, h);
}

QSize SvgIconButton::sizeHint() const {
    QStyleOptionButton option;
    initStyleOption(&option);
    // We draw the label ourselves; ask the style to size a bare button around
    // our own content metrics rather than around its idea of the label.
    option.text.clear();
    option.icon = QIcon();
    option.features &= ~QStyleOptionButton::HasMenu;

    return style()->sizeFromContents(QStyle::CT_PushButton, &option, contentSize(), this);
}

QRect SvgIconButton::contentsRectForPaint(const QStyleOptionButton &option) const {
    QRect r = style()->subElementRect(QStyle::SE_PushButtonContents, &option, this);
    return r.isValid() ? r : rect();
}

void SvgIconButton::setSvgIcon(SvgIcon* icon) {
    if (m_icon && m_icon->parent() == this) {
        m_icon->deleteLater();
    }
    m_icon = icon;
    if (m_icon) {
        m_icon->setParent(this);
        m_icon->hide();
        connect(m_icon, &SvgIcon::visualChanged, this, QOverload<>::of(&QWidget::update));
        connect(m_icon, &SvgIcon::sizeChanged, this, [this]{ updateGeometry(); update(); });
        syncIconState();
    }
    updateGeometry();
    update();
}

void SvgIconButton::setButtonStyle(ButtonStyle style) {
    m_style = style;
    updateStyle();
    updateGeometry();   // borders and padding change the required size
}

void SvgIconButton::makeDropdown(QMenu* menu) {
    setMenu(menu);
    m_menu = menu;
    // The arrow's width is reserved in contentSize(); no stylesheet padding
    // hack is needed, and adding one here would clobber setButtonStyle().
    updateGeometry();
    update();
}

void SvgIconButton::makeToolButton() {
    setButtonStyle(ButtonStyle::Flat);
}

void SvgIconButton::makeToggleable(bool isCheckable) {
    setCheckable(isCheckable);
}

void SvgIconButton::makeMenu(const QList<QAction*>& actions) {
    QMenu* menu = new QMenu(this);
    menu->addActions(actions);
    makeDropdown(menu);
}

void SvgIconButton::updateStyle() {
    // Washes are derived from the palette's highlight, not from a fixed black
    // overlay: rgba(0,0,0,0.08) is invisible on a dark theme. The radius is
    // relative to the type size so it scales with the font and DPI.
    const QColor hl = palette().color(QPalette::Highlight);

    // Idempotent: setStyleSheet() itself triggers a PaletteChange, so bailing
    // out when nothing changed is what stops that from looping forever.
    if (hl == m_styleHighlight && int(m_style) == m_appliedStyle)
        return;
    m_styleHighlight = hl;
    m_appliedStyle = int(m_style);
    const auto wash = [&hl](int alpha) {
        return QStringLiteral("rgba(%1, %2, %3, %4)")
            .arg(hl.red()).arg(hl.green()).arg(hl.blue()).arg(alpha);
    };
    const QString hoverBg   = wash(38);   // ~15%
    const QString pressedBg = wash(76);   // ~30%
    const int radius = qMax(3, em() / 4);

    QString styleSheet;
    switch (m_style) {
        case ButtonStyle::Flat:
            styleSheet = QStringLiteral(
                "SvgIconButton { border: none; background: transparent;"
                "                border-radius: %1px; }"
                "SvgIconButton:hover   { background: %2; }"
                "SvgIconButton:pressed { background: %3; }")
                .arg(radius).arg(hoverBg, pressedBg);
            break;
        case ButtonStyle::Outline:
            styleSheet = QStringLiteral(
                "SvgIconButton { border: 1px solid palette(mid); background: transparent;"
                "                border-radius: %1px; }"
                "SvgIconButton:hover   { background: %2; border-color: palette(highlight); }"
                "SvgIconButton:pressed { background: %3; }")
                .arg(radius).arg(hoverBg, pressedBg);
            break;
        case ButtonStyle::Text:
            styleSheet = QStringLiteral(
                "SvgIconButton { border: none; background: transparent; color: palette(link); }"
                "SvgIconButton:hover   { text-decoration: underline; }"
                "SvgIconButton:pressed { color: palette(highlight); }");
            break;
        case ButtonStyle::Link:
            styleSheet = QStringLiteral(
                "SvgIconButton { border: none; background: transparent;"
                "                color: palette(link); text-decoration: underline; }"
                "SvgIconButton:hover { color: palette(highlight); }");
            break;
        case ButtonStyle::Default:
        default:
            styleSheet = QString();
            break;
    }
    setStyleSheet(styleSheet);
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

// Resolve the icon state from the button's current condition. Disabled beats
// pressed beats hovered beats checked, which matches how users read a control.
void SvgIconButton::syncIconState() {
    if (!m_icon)
        return;

    SvgIcon::State s;
    if (!isEnabled())      s = SvgIcon::Disabled;
    else if (m_isPressed)  s = SvgIcon::Pressed;
    else if (m_isHovered)  s = SvgIcon::Hovered;
    else if (isChecked())  s = SvgIcon::Selected;
    else                   s = SvgIcon::Normal;

    m_icon->setState(s);
}

void SvgIconButton::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    syncIconState();
    QPushButton::enterEvent(event);
}

void SvgIconButton::leaveEvent(QEvent* event) {
    m_isHovered = false;
    syncIconState();
    QPushButton::leaveEvent(event);
}

void SvgIconButton::mousePressEvent(QMouseEvent* event) {
    m_isPressed = true;
    syncIconState();
    update();
    QPushButton::mousePressEvent(event);
}

void SvgIconButton::mouseReleaseEvent(QMouseEvent* event) {
    m_isPressed = false;
    syncIconState();
    update();
    QPushButton::mouseReleaseEvent(event);
}

void SvgIconButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    // Draw only the button chrome (border, background, pressed state). The label
    // is cleared first: CE_PushButton would otherwise paint the text and icon
    // itself, and we paint both below at our own offsets — drawing them twice.
    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    option.text.clear();
    option.icon = QIcon();
    option.features &= ~QStyleOptionButton::HasMenu; // we draw the arrow ourselves
    painter.drawControl(QStyle::CE_PushButton, option);

    // Lay out inside the style's contents rect, so frame width and stylesheet
    // padding are respected instead of guessed at.
    QRect content = contentsRectForPaint(option);

    // Reserve the arrow column first; nothing else may paint into it.
    QRect arrowRect;
    if (m_menu) {
        const int aw = arrowWidth();
        arrowRect = QRect(content.right() - aw + 1,
                          content.center().y() - aw / 2,
                          aw, aw);
        content.setRight(arrowRect.left() - arrowGap());
    }

    if (m_icon) {
        const QSize iconSize = m_icon->size();
        const QRect iconRect(content.left(),
                             content.center().y() - iconSize.height() / 2,
                             iconSize.width(), iconSize.height());

        // Render icon directly via painter translate — avoids geometry mutation.
        // DrawChildren only: QWidget::render() defaults to
        // DrawWindowBackground|DrawChildren, which would fill the icon's rect
        // with the palette's window brush and box every icon in a solid card.
        painter.save();
        painter.translate(iconRect.topLeft());
        m_icon->render(&painter, QPoint(0, 0),
                       QRegion(0, 0, iconSize.width(), iconSize.height()),
                       QWidget::DrawChildren);
        painter.restore();

        content.setLeft(iconRect.right() + 1 + iconTextGap());
    }

    if (!text().isEmpty() && content.width() > 0) {
        // Elide rather than overflow: a button too narrow for its label should
        // degrade legibly, not paint outside its frame.
        const QString shown = painter.fontMetrics().elidedText(
            text(), Qt::ElideRight, content.width());
        painter.drawText(content, Qt::AlignLeft | Qt::AlignVCenter, shown);
    }

    if (m_menu) {
        QStyleOption arrowOpt;
        arrowOpt.initFrom(this);
        arrowOpt.rect = arrowRect;
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, &painter, this);
    }
}

bool SvgIconButton::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::EnabledChange:
            syncIconState();
            break;
        case QEvent::PaletteChange:
        case QEvent::ApplicationPaletteChange:
            // The hover/pressed washes derive from the palette highlight, so a
            // theme switch must regenerate them. updateStyle() is idempotent,
            // which is what keeps setStyleSheet()'s own PaletteChange from
            // recursing back into here.
            updateStyle();
            updateGeometry();
            break;
        case QEvent::FontChange:
        case QEvent::StyleChange:
            updateGeometry();   // text metrics / frame width just changed
            break;
        default:
            break;
    }
    return QPushButton::event(event);
}
