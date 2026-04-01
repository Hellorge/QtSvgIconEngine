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

    if (m_icon) {
        // Transfer ownership to this widget so Qt manages lifetime
        m_icon->setParent(this);
        // Hide the icon widget — we render it manually in paintEvent
        m_icon->hide();

        const QSize iconSize = m_icon->size();
        if (text().isEmpty()) {
            setFixedSize(iconSize);
        } else {
            // Icon + padding + text
            QFontMetrics fm(font());
            const int textWidth = fm.horizontalAdvance(text());
            setFixedSize(iconSize.width() + 8 + textWidth + 16,
                         qMax(iconSize.height(), fm.height()) + 8);
        }
    }
}

void SvgIconButton::setSvgIcon(SvgIcon* icon) {
    if (m_icon && m_icon->parent() == this) {
        m_icon->deleteLater();
    }
    m_icon = icon;
    if (m_icon) {
        m_icon->setParent(this);
        m_icon->hide();
        setFixedSize(m_icon->size());
        update();
    }
}

void SvgIconButton::setButtonStyle(ButtonStyle style) {
    m_style = style;
    updateStyle();
}

void SvgIconButton::makeDropdown(QMenu* menu) {
    setMenu(menu);
    m_menu = menu;
    setupDropdownArrow();
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

void SvgIconButton::setupDropdownArrow() {
    setStyleSheet(R"(
        SvgIconButton {
            padding-right: 18px;
        }
    )");
}

void SvgIconButton::updateStyle() {
    QString styleSheet;
    switch (m_style) {
        case ButtonStyle::Flat:
            styleSheet = R"(
                SvgIconButton {
                    border: none;
                    background: transparent;
                    border-radius: 4px;
                }
                SvgIconButton:hover {
                    background: rgba(0, 0, 0, 0.08);
                }
                SvgIconButton:pressed {
                    background: rgba(0, 0, 0, 0.18);
                }
            )";
            break;
        case ButtonStyle::Outline:
            styleSheet = R"(
                SvgIconButton {
                    border: 1px solid palette(mid);
                    background: transparent;
                    border-radius: 4px;
                }
                SvgIconButton:hover {
                    background: rgba(0, 0, 0, 0.06);
                    border-color: palette(dark);
                }
                SvgIconButton:pressed {
                    background: rgba(0, 0, 0, 0.14);
                }
            )";
            break;
        case ButtonStyle::Text:
            styleSheet = R"(
                SvgIconButton {
                    border: none;
                    background: transparent;
                    color: palette(link);
                }
                SvgIconButton:hover {
                    text-decoration: underline;
                }
                SvgIconButton:pressed {
                    color: palette(highlight);
                }
            )";
            break;
        case ButtonStyle::Link:
            styleSheet = R"(
                SvgIconButton {
                    border: none;
                    background: transparent;
                    color: palette(link);
                    text-decoration: underline;
                }
                SvgIconButton:hover {
                    color: palette(highlight);
                }
            )";
            break;
        case ButtonStyle::Default:
        default:
            styleSheet = "";
            break;
    }
    setStyleSheet(styleSheet);
}

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

void SvgIconButton::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    if (m_icon)
        m_icon->setState(SvgIcon::Active);
    QPushButton::enterEvent(event);
}

void SvgIconButton::leaveEvent(QEvent* event) {
    m_isHovered = false;
    if (m_icon)
        m_icon->setState(SvgIcon::Normal);
    QPushButton::leaveEvent(event);
}

void SvgIconButton::mousePressEvent(QMouseEvent* event) {
    m_isPressed = true;
    update();
    QPushButton::mousePressEvent(event);
}

void SvgIconButton::mouseReleaseEvent(QMouseEvent* event) {
    m_isPressed = false;
    update();
    QPushButton::mouseReleaseEvent(event);
}

void SvgIconButton::paintEvent(QPaintEvent* event) {
    // Draw standard button chrome (border, background, pressed state)
    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    painter.drawControl(QStyle::CE_PushButton, option);

    if (!m_icon)
        return;

    const QSize iconSize = m_icon->size();
    const int padding = 4;

    // Icon rect — left-aligned with padding
    QRect iconRect(padding, (height() - iconSize.height()) / 2,
                   iconSize.width(), iconSize.height());

    // Render icon directly via painter translate — avoids geometry mutation
    painter.save();
    painter.translate(iconRect.topLeft());
    m_icon->render(&painter, QPoint(0, 0),
                   QRegion(0, 0, iconSize.width(), iconSize.height()));
    painter.restore();

    // Text — positioned after icon
    if (!text().isEmpty()) {
        QRect textRect = rect();
        textRect.setLeft(iconRect.right() + padding * 2);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    }

    // Dropdown arrow
    if (m_menu) {
        QStyleOption arrowOpt;
        arrowOpt.initFrom(this);
        arrowOpt.rect = QRect(width() - 14, (height() - 8) / 2, 8, 8);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, &painter, this);
    }
}

bool SvgIconButton::event(QEvent* event) {
    if (event->type() == QEvent::EnabledChange && m_icon) {
        m_icon->setState(isEnabled() ? SvgIcon::Normal : SvgIcon::Disabled);
    }
    return QPushButton::event(event);
}
