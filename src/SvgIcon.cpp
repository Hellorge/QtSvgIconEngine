// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIcon.h"
#include "SvgIconPainter.h"
#include "SvgStroke.h"
#include <QApplication>
#include <QEvent>

SvgIcon::SvgIcon(const QSharedPointer<QSvgRenderer> &renderer, QVariantMap &options,
                 QWidget *parent)
    : QSvgWidget(parent), m_renderer(renderer), m_baseRenderer(renderer) {
    setOptions(options);
    updateCachedImage();
}

SvgIcon::~SvgIcon() {
    // m_renderer is a QSharedPointer — the renderer is released here only if
    // this was the last reference (the engine's cache may still hold one).
}

// ---------------------------------------------------------------------------
// Option resolution
// ---------------------------------------------------------------------------

const char* SvgIcon::statePrefix(State state) {
    switch (state) {
        case Hovered:  return "hover_";
        case Pressed:  return "pressed_";
        case Selected: return "selected_";
        case Disabled: return "disabled_";
        case Normal:
        default:       return "";
    }
}

// An explicitly supplied per-state override, or an invalid QVariant if absent.
QVariant SvgIcon::stateOverride(const QVariantMap &options, State state, const QString &key) {
    if (state == Normal)
        return QVariant();
    const QVariant v = options.value(QString::fromLatin1(statePrefix(state)) + key);
    return v.isValid() ? v : QVariant();
}

// An absent colour key means "follow the palette".
QColor SvgIcon::inheritedColor(const QVariantMap &options, const QString &key) {
    const QVariant v = options.value(key);
    if (v.isValid())
        return v.value<QColor>();
    return QApplication::palette().text().color();
}

void SvgIcon::setOptions(const QVariantMap &options) {
    m_options = options;

    // Absent colour keys mean "follow the palette" — remember that so a theme
    // change can re-resolve them.
    m_colorInherited       = !options.value(QStringLiteral("color")).isValid();
    m_borderColorInherited = !options.value(QStringLiteral("border_color")).isValid();

    m_defaultColors = options.value(QStringLiteral("default_colors")).toBool();

    if (options.contains(QStringLiteral("transition_ms")))
        m_transitionMs = options.value(QStringLiteral("transition_ms")).toInt();

    setFixedSize(options.value(QStringLiteral("size")).toSize());

    m_state = Normal;
    applyOptions(optionsForState(Normal));
}

QVariantMap SvgIcon::optionsForState(State state) const {
    return resolveOptions(m_options, state);
}

QVariantMap SvgIcon::resolveOptions(const QVariantMap &options, State state) {
    QVariantMap t;
    const QColor base       = inheritedColor(options, QStringLiteral("color"));
    const QColor baseBorder = inheritedColor(options, QStringLiteral("border_color"));
    const qreal  baseOpacity = options.value(QStringLiteral("opacity"), 1.0).toReal();
    const qreal  baseScale   = options.value(QStringLiteral("scale"), 1.0).toReal();
    const qreal  baseBorderW = options.value(QStringLiteral("border_width"), 0.0).toReal();
    const QColor baseBg      = options.value(QStringLiteral("background"),
                                             QColor(Qt::transparent)).value<QColor>();

    t[QStringLiteral("default_colors")] = options.value(QStringLiteral("default_colors")).toBool();

    // How each state derives from Normal when it has no explicit override.
    // These are relative factors, not fixed colours, so a theme change or a new
    // base colour carries through automatically.
    const int   hoverLighten   = options.value(QStringLiteral("hover_lighten"),   130).toInt();
    const int   pressedDarken  = options.value(QStringLiteral("pressed_darken"),  115).toInt();
    const qreal disabledFactor = options.value(QStringLiteral("disabled_opacity_factor"), 0.5).toReal();
    const int   selectedWash   = options.value(QStringLiteral("selected_wash_alpha"), 60).toInt();

    // colour — explicit override wins, else a sensible per-state derivation
    QVariant v;
    if ((v = stateOverride(options, state, QStringLiteral("color"))).isValid()) {
        t[QStringLiteral("color")] = v;
    } else {
        switch (state) {
            case Hovered: t[QStringLiteral("color")] = base.lighter(hoverLighten); break;
            case Pressed: t[QStringLiteral("color")] = base.darker(pressedDarken); break;
            default:      t[QStringLiteral("color")] = base;                       break;
        }
    }

    // background — Selected gets a translucent highlight wash by default
    if ((v = stateOverride(options, state, QStringLiteral("background"))).isValid()) {
        t[QStringLiteral("background")] = v;
    } else if (state == Selected) {
        QColor hl = QApplication::palette().highlight().color();
        hl.setAlpha(selectedWash);
        t[QStringLiteral("background")] = hl;
    } else {
        t[QStringLiteral("background")] = baseBg;
    }

    // opacity — Disabled dims by default
    if ((v = stateOverride(options, state, QStringLiteral("opacity"))).isValid())
        t[QStringLiteral("opacity")] = v;
    else
        t[QStringLiteral("opacity")] = (state == Disabled) ? baseOpacity * disabledFactor : baseOpacity;

    if ((v = stateOverride(options, state, QStringLiteral("scale"))).isValid())
        t[QStringLiteral("scale")] = v;
    else
        t[QStringLiteral("scale")] = baseScale;

    if ((v = stateOverride(options, state, QStringLiteral("border_color"))).isValid())
        t[QStringLiteral("border_color")] = v;
    else
        t[QStringLiteral("border_color")] = baseBorder;

    if ((v = stateOverride(options, state, QStringLiteral("border_width"))).isValid())
        t[QStringLiteral("border_width")] = v;
    else
        t[QStringLiteral("border_width")] = baseBorderW;

    if ((v = stateOverride(options, state, QStringLiteral("stroke_progress"))).isValid())
        t[QStringLiteral("stroke_progress")] = v;
    else
        t[QStringLiteral("stroke_progress")] =
            options.value(QStringLiteral("stroke_progress"), 1.0).toReal();

    return t;
}

// Live values, which may be part-way through a transition — this is what the
// widget actually paints, as opposed to the target of the current animation.
QVariantMap SvgIcon::currentOptions() const {
    QVariantMap o;
    o[QStringLiteral("color")]          = m_color;
    o[QStringLiteral("background")]     = m_background;
    o[QStringLiteral("opacity")]        = m_opacity;
    o[QStringLiteral("scale")]          = m_scale;
    o[QStringLiteral("border_color")]   = m_borderColor;
    o[QStringLiteral("border_width")]   = m_borderWidth;
    o[QStringLiteral("default_colors")] = m_defaultColors;
    return o;
}

void SvgIcon::applyOptions(const QVariantMap &o) {
    m_color       = o.value(QStringLiteral("color")).value<QColor>();
    m_background  = o.value(QStringLiteral("background")).value<QColor>();
    m_opacity     = o.value(QStringLiteral("opacity")).toReal();
    m_scale       = o.value(QStringLiteral("scale")).toReal();
    m_borderColor = o.value(QStringLiteral("border_color")).value<QColor>();
    m_borderWidth = o.value(QStringLiteral("border_width")).toReal();
    if (o.contains(QStringLiteral("stroke_progress")))
        setStrokeProgress(o.value(QStringLiteral("stroke_progress")).toReal());
    update();
    emit visualChanged();
}

// ---------------------------------------------------------------------------
// Property accessors
// ---------------------------------------------------------------------------

QColor SvgIcon::color() const { return m_color; }

void SvgIcon::setColor(const QColor &color) {
    if (m_color != color) {
        m_color = color;
        update();
        emit visualChanged();
    }
}

QColor SvgIcon::background() const { return m_background; }

void SvgIcon::setBackground(const QColor &background) {
    if (m_background != background) {
        m_background = background;
        update();
        emit visualChanged();
    }
}

qreal SvgIcon::opacity() const { return m_opacity; }

void SvgIcon::setOpacity(const qreal opacity) {
    if (m_opacity != opacity) {
        m_opacity = opacity;
        update();
        emit visualChanged();
    }
}

void SvgIcon::setSize(const QSize &size) {
    if (size != this->size()) {
        setFixedSize(size);
        updateCachedImage();
        update();
        emit visualChanged();
        emit sizeChanged(size);   // hosts must re-run layout, not just repaint
    }
}

qreal SvgIcon::scale() const { return m_scale; }

void SvgIcon::setScale(const qreal scale) {
    if (m_scale != scale) {
        m_scale = scale;
        update();
        emit visualChanged();
    }
}

QColor SvgIcon::borderColor() const { return m_borderColor; }

void SvgIcon::setBorderColor(const QColor &borderColor) {
    if (m_borderColor != borderColor) {
        m_borderColor = borderColor;
        update();
        emit visualChanged();
    }
}

// ---------------------------------------------------------------------------
// Stroke effects
// ---------------------------------------------------------------------------

void SvgIcon::setStrokeSource(const QByteArray &svg, qreal strokeLength) {
    m_strokeSource = svg;
    m_strokeLength = strokeLength;
    // The constructor may already have applied a stroke_progress option before
    // the source arrived, in which case rebuilding is not enough — the cached
    // bitmap still holds the un-effected artwork.
    rebuildEffectRenderer();
    updateCachedImage();
    update();
    emit visualChanged();
}

// An effect renderer only exists while an effect is actually doing something;
// otherwise we render the shared, cached renderer and pay nothing.
void SvgIcon::rebuildEffectRenderer() {
    const bool drawing = m_strokeLength > 0.0 && m_strokeProgress < 1.0;
    const bool dashing = m_strokeLength > 0.0 && m_dashPattern > 0.0;

    if (m_strokeSource.isEmpty() || (!drawing && !dashing)) {
        m_effectRenderer.reset();
        return;
    }

    qreal dash, offset;
    if (dashing) {
        dash   = m_dashPattern;
        offset = m_dashOffset;
    } else {
        // dasharray == the whole stroke, offset walks it back to zero
        dash   = m_strokeLength;
        offset = m_strokeLength * (1.0 - qBound(0.0, m_strokeProgress, 1.0));
    }

    m_effectRenderer.reset(new QSvgRenderer(
        SvgStroke::injectDash(m_strokeSource, dash, offset)));
    if (!m_effectRenderer->isValid())
        m_effectRenderer.reset();
}

QSvgRenderer *SvgIcon::activeRenderer() const {
    return m_effectRenderer ? m_effectRenderer.data() : m_renderer.data();
}

void SvgIcon::setStrokeProgress(qreal progress) {
    progress = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(m_strokeProgress, progress))
        return;
    m_strokeProgress = progress;
    rebuildEffectRenderer();
    updateCachedImage();
    update();
    emit visualChanged();
}

void SvgIcon::setDashPattern(qreal dash) {
    if (qFuzzyCompare(m_dashPattern, dash))
        return;
    m_dashPattern = qMax(0.0, dash);
    rebuildEffectRenderer();
    updateCachedImage();
    update();
    emit visualChanged();
}

void SvgIcon::setDashOffset(qreal offset) {
    if (qFuzzyCompare(m_dashOffset, offset))
        return;
    m_dashOffset = offset;
    if (m_dashPattern > 0.0) {
        rebuildEffectRenderer();
        updateCachedImage();
        update();
        emit visualChanged();
    }
}

qreal SvgIcon::borderWidth() const { return m_borderWidth; }

void SvgIcon::setBorderWidth(const qreal borderWidth) {
    if (m_borderWidth != borderWidth) {
        m_borderWidth = borderWidth;
        update();
        emit visualChanged();
    }
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

void SvgIcon::setStateRenderer(State state, const QSharedPointer<QSvgRenderer> &renderer) {
    if (renderer)
        m_stateRenderers.insert(state, renderer);
    else
        m_stateRenderers.remove(state);

    if (state == m_state) {
        m_renderer = renderer ? renderer : m_baseRenderer;
        updateCachedImage();
        update();
        emit visualChanged();
    }
}

void SvgIcon::setState(State state) {
    if (m_state == state)
        return;
    m_state = state;

    // Swap artwork if this state has its own. Geometry is unchanged, so only
    // the cached bitmap needs rebuilding.
    const QSharedPointer<QSvgRenderer> wanted =
        m_stateRenderers.value(state, m_baseRenderer);
    if (wanted != m_renderer) {
        m_renderer = wanted;
        updateCachedImage();
    }

    const QVariantMap target = optionsForState(state);
    if (m_transitionMs <= 0)
        applyOptions(target);          // instant, matches pre-animation behaviour
    else
        animateTo(target, m_transitionMs, m_easing);
}

// ---------------------------------------------------------------------------
// Palette
// ---------------------------------------------------------------------------

void SvgIcon::changeEvent(QEvent *event) {
    const QEvent::Type t = event->type();
    if (t == QEvent::PaletteChange || t == QEvent::ApplicationPaletteChange) {
        // Only icons that never had an explicit colour follow the theme.
        if (m_colorInherited || m_borderColorInherited)
            applyOptions(optionsForState(m_state));
    }
    QSvgWidget::changeEvent(event);
}

// ---------------------------------------------------------------------------
// Device pixel ratio — taken from the screen, never set by hand
// ---------------------------------------------------------------------------

bool SvgIcon::event(QEvent *e) {
    // Moving to a screen with a different scale factor must re-rasterise, or a
    // 1x bitmap gets stretched across 2x pixels and the icon looks soft.
    //
    // DevicePixelRatioChange arrived in Qt 6.6. On 6.5 the icon still rasterises
    // at the ratio it was created with; it just will not follow a move between
    // differently-scaled screens.
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    if (e->type() == QEvent::DevicePixelRatioChange) {
        updateCachedImage();
        update();
    }
#endif
    return QSvgWidget::event(e);
}

// ---------------------------------------------------------------------------
// Sprite element
// ---------------------------------------------------------------------------

void SvgIcon::setElementId(QString elementId) {
    if (m_elementId != elementId) {
        m_elementId = elementId;
        updateCachedImage();
        update();
        emit visualChanged();
    }
}

// ---------------------------------------------------------------------------
// Cache
// ---------------------------------------------------------------------------

void SvgIcon::updateCachedImage() {
    // Cached uncoloured; tinting happens per-paint so animating colour does not
    // force a re-rasterise of the SVG on every frame. The raster resolution
    // follows the screen, so the icon stays crisp on HiDPI displays.
    m_cachedImage = SvgIconPainter::rasterize(activeRenderer(), size(),
                                              devicePixelRatioF(), m_elementId);
}

// ---------------------------------------------------------------------------
// animateTo — convenience wrapper around QPropertyAnimation
// ---------------------------------------------------------------------------

void SvgIcon::animateTo(const QVariantMap &targetOptions, int durationMs,
                        QEasingCurve::Type easing) {
    // A rapid hover in/out would otherwise stack animation groups that keep
    // fighting over the same properties.
    if (m_transition) {
        m_transition->stop();
        m_transition->deleteLater();
    }

    static const QList<QByteArray> animatableProps = {
        "color", "background", "opacity", "scale",
        "border_color", "border_width", "size",
        "stroke_progress", "dash_offset"
    };

    auto *group = new QParallelAnimationGroup(this);

    for (const QByteArray &prop : animatableProps) {
        if (!targetOptions.contains(prop))
            continue;

        QVariant target = targetOptions.value(prop);
        if (!target.isValid())
            continue;

        auto *anim = new QPropertyAnimation(this, prop, group);
        anim->setDuration(durationMs);
        anim->setEndValue(target);
        anim->setEasingCurve(easing);
        group->addAnimation(anim);
    }

    m_transition = group;
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void SvgIcon::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    if (m_cachedImage.isNull())
        return;

    QPainter painter(this);
    SvgIconPainter::composite(&painter, rect(), m_cachedImage, currentOptions());
}
