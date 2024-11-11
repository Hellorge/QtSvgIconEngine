// SvgIconButton.h
#ifndef SVGICONBUTTON_H
#define SVGICONBUTTON_H

#include <QPushButton>
#include <QMenu>
#include <QAction>
#include "SvgIcon.h"

class SvgIconButton : public QPushButton {
    Q_OBJECT

public:
    explicit SvgIconButton(SvgIcon* icon, QWidget* parent = nullptr);
    explicit SvgIconButton(SvgIcon* icon, const QString& text, QWidget* parent = nullptr);
    ~SvgIconButton();

    // Icon management
    SvgIcon* svgIcon() const { return m_icon; }
    void setSvgIcon(SvgIcon* icon);

    // Style management
    enum ButtonStyle {
        Default,
        Flat,
        Outline,
        Text,
        Link
    };
    void setButtonStyle(ButtonStyle style);

    // Convenience methods for common button configurations
    void makeDropdown(QMenu* menu);
    void makeToolButton();
    void makeToggleable(bool isCheckable = true);
    void makeMenu(const QList<QAction*>& actions);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;

private:
    SvgIcon* m_icon;
    ButtonStyle m_style;
    QMenu* m_menu;
    bool m_isPressed;
    bool m_isHovered;

    void init();
    void updateStyle();
    void setupDropdownArrow();
};

#endif // SVGICONBUTTON_H
