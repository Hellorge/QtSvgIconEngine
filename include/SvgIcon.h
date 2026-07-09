#ifndef SVGICON_H
#define SVGICON_H

#include <QSvgWidget>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QEasingCurve>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QHash>
#include <QImage>
#include <QSharedPointer>
#include <QStyle>
#include <QStyleOptionButton>

class SvgIcon : public QSvgWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QSize size READ size WRITE setSize)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(QColor border_color READ borderColor WRITE setBorderColor)
    Q_PROPERTY(qreal border_width READ borderWidth WRITE setBorderWidth)
    Q_PROPERTY(qreal stroke_progress READ strokeProgress WRITE setStrokeProgress)
    Q_PROPERTY(qreal dash_offset READ dashOffset WRITE setDashOffset)

public:
    // Interaction states. Each may override any visual option via a prefixed
    // key ("hover_color", "pressed_scale", ...) and may carry its own SVG via
    // "hover_icon" and friends. Active is kept as an alias for source compat.
    enum State {
        Normal   = 0,
        Hovered  = 1,
        Pressed  = 2,
        Selected = 3,
        Disabled = 4,
        Active   = Hovered
    };

    SvgIcon(const QSharedPointer<QSvgRenderer> &renderer, QVariantMap &options,
            QWidget *parent = nullptr);
    ~SvgIcon();

    QColor color() const;
    void setColor(const QColor &color);

    QColor background() const;
    void setBackground(const QColor &background);

    qreal opacity() const;
    void setOpacity(const qreal opacity);

    // size() is inherited from QWidget, setSize wraps setFixedSize
    void setSize(const QSize &size);

    qreal scale() const;
    void setScale(const qreal scale);

    QColor borderColor() const;
    void setBorderColor(const QColor &borderColor);

    qreal borderWidth() const;
    void setBorderWidth(const qreal borderWidth);

    // 0 = undrawn, 1 = fully drawn. Animating this "writes" the icon on, the way
    // stroke-dashoffset does on the web. Requires stroked artwork; a filled icon
    // has no stroke to draw and ignores it.
    qreal strokeProgress() const { return m_strokeProgress; }
    void setStrokeProgress(qreal progress);

    // Marching-ants offset, in user units. Animate it for a moving dash.
    // Only meaningful when dashPattern() is set.
    qreal dashOffset() const { return m_dashOffset; }
    void setDashOffset(qreal offset);
    void setDashPattern(qreal dash);          // 0 disables
    qreal dashPattern() const { return m_dashPattern; }

    // The length of the longest subpath, or 0 when the artwork has no stroke.
    qreal strokeLength() const { return m_strokeLength; }

    // Supplies the source needed by the stroke effects above. The engine calls
    // this; there is no need to do so by hand.
    void setStrokeSource(const QByteArray &svg, qreal strokeLength);

    // State changes animate over setTransitionDuration() milliseconds.
    // Set it to 0 for an instant switch.
    void setState(State state);
    State state() const { return m_state; }

    void setTransitionDuration(int ms) { m_transitionMs = ms; }

    // Give a state its own artwork. Ownership is shared; pass a renderer
    // obtained from SvgIconEngine. Passing a null renderer clears the override.
    void setStateRenderer(State state, const QSharedPointer<QSvgRenderer> &renderer);
    bool hasStateRenderer(State state) const { return m_stateRenderers.contains(state); }

    void setElementId(QString elementId);

    // Borrowed pointer, valid for the lifetime of this SvgIcon.
    QSvgRenderer* renderer() const { return m_renderer.data(); }

    // Animate any combination of properties to target values. Any previously
    // running transition is stopped first.
    // Animatable keys: "color", "background", "opacity", "scale",
    //                  "border_color", "border_width", "size",
    //                  "stroke_progress", "dash_offset"
    void animateTo(const QVariantMap &targetOptions, int durationMs = 300,
                   QEasingCurve::Type easing = QEasingCurve::InOutQuad);

    // Pure resolution of an option map for a state. Shared with SvgQIconEngine
    // so widget and QIcon rendering agree on what each state looks like.
    static QVariantMap resolveOptions(const QVariantMap &options, State state);

signals:
    // Emitted whenever the icon's appearance changes. Hosts that render an
    // SvgIcon manually (SvgIconButton hides it and calls render()) must connect
    // this to their own update(), or animation frames will never be painted.
    void visualChanged();

    // Emitted only when the icon's geometry changes. Kept separate from
    // visualChanged() so hosts don't re-run layout on every animation frame.
    void sizeChanged(const QSize &size);

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool event(QEvent *event) override;

private:
    // Ownership is shared with SvgIconEngine's cache. The renderer outlives cache
    // eviction for as long as any SvgIcon still references it.
    QSharedPointer<QSvgRenderer> m_renderer;      // currently displayed
    QSharedPointer<QSvgRenderer> m_baseRenderer;  // the Normal-state artwork
    QHash<int, QSharedPointer<QSvgRenderer>> m_stateRenderers;
    QImage m_cachedImage;

    // Stroke effects re-render the SVG source with injected dash attributes, so
    // they need the bytes, not just a parsed renderer.
    QByteArray m_strokeSource;
    QSharedPointer<QSvgRenderer> m_effectRenderer;  // non-null while an effect is active
    qreal m_strokeLength   = 0.0;
    qreal m_strokeProgress = 1.0;
    qreal m_dashPattern    = 0.0;
    qreal m_dashOffset     = 0.0;

    // Full option map as resolved by the engine, including per-state keys.
    QVariantMap m_options;

    QColor m_color;
    QColor m_background;
    qreal m_opacity        = 1.0;
    qreal m_scale          = 1.0;
    QColor m_borderColor;
    qreal m_borderWidth    = 0.0;
    bool m_defaultColors   = false;
    QString m_elementId;

    // When the caller never supplied an explicit colour we inherit it from the
    // application palette, and must re-resolve it whenever the theme changes.
    bool m_colorInherited       = true;
    bool m_borderColorInherited = true;

    State m_state            = Normal;
    int m_transitionMs       = 150;
    QEasingCurve::Type m_easing = QEasingCurve::InOutQuad;
    QPointer<QParallelAnimationGroup> m_transition;

    static const char* statePrefix(State state);
    static QVariant stateOverride(const QVariantMap &options, State state, const QString &key);
    static QColor inheritedColor(const QVariantMap &options, const QString &key);

    QVariantMap optionsForState(State state) const;
    QVariantMap currentOptions() const;  // live (possibly mid-animation) values

    void applyOptions(const QVariantMap &options); // instant, no animation
    void rebuildEffectRenderer();
    QSvgRenderer *activeRenderer() const;
    void updateCachedImage();
    void setOptions(const QVariantMap &options);
};

#endif // SVGICON_H
