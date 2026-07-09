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

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool event(QEvent* event) override;

private:
    // Layout metrics are relative, not absolute. Everything is expressed in
    // fractions of `em` (the font's line height) or asked of the current
    // QStyle, so the button scales correctly with font size, DPI and theme
    // instead of being tuned for one screen. Frame/padding comes from the style.
    int em() const;                 // the type-relative unit
    int iconTextGap() const;        // between icon and label
    int textPadding() const;        // trailing breathing room after the label
    int verticalPadding() const;    // keeps a styled border off the icon
    int arrowWidth() const;         // asked of the style, not guessed
    int arrowGap() const;           // between label and dropdown arrow

    SvgIcon* m_icon;
    ButtonStyle m_style;
    QMenu* m_menu;
    bool m_isPressed;
    bool m_isHovered;
    // setStyleSheet() re-applies the widget palette, which posts another
    // PaletteChange. Rebuilding the sheet on that event would recurse, so
    // updateStyle() is idempotent: it only touches the stylesheet when the
    // inputs it derives from have actually changed.
    QColor m_styleHighlight;
    int m_appliedStyle = -1;

    void init();
    void updateStyle();
    void syncIconState();

    QRect contentsRectForPaint(const QStyleOptionButton &option) const;
    QSize contentSize() const;   // icon + gap + text + arrow, before style padding
};

#endif // SVGICONBUTTON_H
