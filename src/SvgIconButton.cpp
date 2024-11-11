// SvgIconButton.cpp
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
    // m_icon is owned by SvgIconEngine, don't delete it here
}

void SvgIconButton::init() {
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    if (m_icon) {
        setFixedSize(m_icon->size());
        m_icon->setParent(this);
    }

    // Enable styling through Qt Style Sheets
    setAttribute(Qt::WA_StyledBackground, true);
}

void SvgIconButton::setSvgIcon(SvgIcon* icon) {
    m_icon = icon;
    if (m_icon) {
        setFixedSize(m_icon->size());
        m_icon->setParent(this);
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
    // Additional tool button specific setup
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
            padding-right: 15px;  /* Space for the arrow */
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
                }
                SvgIconButton:hover {
                    background: rgba(0, 0, 0, 0.1);
                }
                SvgIconButton:pressed {
                    background: rgba(0, 0, 0, 0.2);
                }
            )";
            break;
        case ButtonStyle::Outline:
            styleSheet = R"(
                SvgIconButton {
                    border: 1px solid palette(button);
                    background: transparent;
                }
                SvgIconButton:hover {
                    background: rgba(0, 0, 0, 0.1);
                }
            )";
            break;
        // Add more styles as needed
        default:
            styleSheet = "";
    }
    setStyleSheet(styleSheet);
}

void SvgIconButton::enterEvent(QEnterEvent* event) {
    m_isHovered = true;
    if (m_icon) {
        m_icon->setState(SvgIcon::Active);
    }
    QPushButton::enterEvent(event);
}

void SvgIconButton::leaveEvent(QEvent* event) {
    m_isHovered = false;
    if (m_icon) {
        m_icon->setState(SvgIcon::Normal);
    }
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
    QStylePainter painter(this);
    QStyleOptionButton option;
    initStyleOption(&option);

    // Draw the button background/frame
    painter.drawControl(QStyle::CE_PushButton, option);

    // Draw the SVG icon
    if (m_icon) {
        QRect iconRect = rect();
        if (!text().isEmpty()) {
            // Adjust icon position if there's text
            iconRect.setWidth(m_icon->width());
            iconRect.moveLeft(5); // Add some padding
        }

        // Save the icon's original geometry
        QRect originalGeometry = m_icon->geometry();

        // Temporarily move the icon to where we want to paint it
        m_icon->setGeometry(iconRect);

        // Paint the icon
        m_icon->render(&painter);

        // Restore the icon's original position
        m_icon->setGeometry(originalGeometry);
    }

    // Draw the dropdown arrow if this is a menu button
    if (m_menu) {
        QStyleOption arrowOpt;
        arrowOpt.initFrom(this);
        arrowOpt.rect = QRect(width() - 15, height() / 2 - 3, 9, 9);
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, &painter, this);
    }

    // Draw the text if any
    if (!text().isEmpty()) {
        QRect textRect = rect();
        if (m_icon) {
            textRect.setLeft(m_icon->width() + 10); // Add padding after icon
        }
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
    }
}

bool SvgIconButton::event(QEvent* event) {
    switch (event->type()) {
        case QEvent::EnabledChange:
            if (m_icon) {
                m_icon->setState(isEnabled() ? SvgIcon::Normal : SvgIcon::Disabled);
            }
            break;
        default:
            break;
    }
    return QPushButton::event(event);
}
