// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIcon.h"

SvgIcon::SvgIcon(QSvgRenderer *renderer, QVariantMap &options, QWidget *parent)
    : QSvgWidget(parent), m_renderer(renderer) {
    setOptions(options);
    updateCachedImage();
    m_devicePixelRatio = 1.0;
}

SvgIcon::~SvgIcon() {
    // m_renderer is owned by SvgIconEngine's cache — do NOT delete here.
}

void SvgIcon::setOptions(const QVariantMap &options) {
    m_color         = options.value("color").value<QColor>();
    m_originalColor = m_color; // preserve for setState restore
    m_background    = options.value("background").value<QColor>();
    m_opacity       = options.value("opacity").toReal();
    m_scale         = options.value("scale").toReal();
    m_borderColor   = options.value("border_color").value<QColor>();
    m_borderWidth   = options.value("border_width").toReal();
    m_defaultColors = options.value("default_colors").toBool();
    m_state         = Normal;
    setFixedSize(options.value("size").toSize());
}

const QVariantMap SvgIcon::getOptions() const {
    QVariantMap map;
    map.insert("color",        color());
    map.insert("background",   background());
    map.insert("opacity",      opacity());
    map.insert("scale",        scale());
    map.insert("border_color", borderColor());
    map.insert("border_width", borderWidth());
    map.insert("default_colors", m_defaultColors);
    map.insert("size",         size());
    return map;
}

// ---------------------------------------------------------------------------
// Property accessors
// ---------------------------------------------------------------------------

QColor SvgIcon::color() const { return m_color; }

void SvgIcon::setColor(const QColor &color) {
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor SvgIcon::background() const { return m_background; }

void SvgIcon::setBackground(const QColor &background) {
    if (m_background != background) {
        m_background = background;
        update();
    }
}

qreal SvgIcon::opacity() const { return m_opacity; }

void SvgIcon::setOpacity(const qreal opacity) {
    if (m_opacity != opacity) {
        m_opacity = opacity;
        update();
    }
}

void SvgIcon::setSize(const QSize &size) {
    if (size != this->size()) {
        setFixedSize(size);
        updateCachedImage();
        update();
    }
}

qreal SvgIcon::scale() const { return m_scale; }

void SvgIcon::setScale(const qreal scale) {
    if (m_scale != scale) {
        m_scale = scale;
        update();
    }
}

QColor SvgIcon::borderColor() const { return m_borderColor; }

void SvgIcon::setBorderColor(const QColor &borderColor) {
    if (m_borderColor != borderColor) {
        m_borderColor = borderColor;
        update();
    }
}

qreal SvgIcon::borderWidth() const { return m_borderWidth; }

void SvgIcon::setBorderWidth(const qreal borderWidth) {
    if (m_borderWidth != borderWidth) {
        m_borderWidth = borderWidth;
        update();
    }
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

void SvgIcon::setState(State state) {
    if (m_state != state) {
        m_state = state;
        updateStateStyle();
    }
}

void SvgIcon::updateStateStyle() {
    switch (m_state) {
        case Disabled:
            setOpacity(0.5);
            break;
        case Active:
            // Use a tinted version of the original color rather than hardcoded blue
            setColor(m_originalColor.lighter(150));
            break;
        case Selected:
            setBackground(Qt::lightGray);
            break;
        case Normal:
        default:
            setOpacity(1.0);
            setColor(m_originalColor); // restore original, not m_color which may be mutated
            setBackground(Qt::transparent);
            break;
    }
    update();
}

// ---------------------------------------------------------------------------
// Device pixel ratio
// ---------------------------------------------------------------------------

void SvgIcon::setDevicePixelRatio(qreal dpr) {
    if (m_devicePixelRatio != dpr) {
        m_devicePixelRatio = dpr;
        updateScaledSize();
    }
}

void SvgIcon::updateScaledSize() {
    QSize scaledSize = size() * m_devicePixelRatio;
    m_cachedImage = QImage(scaledSize, QImage::Format_ARGB32_Premultiplied);
    m_cachedImage.setDevicePixelRatio(m_devicePixelRatio);
    updateCachedImage();
    update();
}

// ---------------------------------------------------------------------------
// Sprite element
// ---------------------------------------------------------------------------

void SvgIcon::setElementId(QString elementId) {
    if (m_elementId != elementId) {
        m_elementId = elementId;
        update();
    }
}

// ---------------------------------------------------------------------------
// Cache
// ---------------------------------------------------------------------------

void SvgIcon::updateCachedImage() {
    if (!m_renderer || !m_renderer->isValid())
        return;

    m_cachedImage = QImage(size() * m_devicePixelRatio, QImage::Format_ARGB32_Premultiplied);
    m_cachedImage.setDevicePixelRatio(m_devicePixelRatio);
    m_cachedImage.fill(Qt::transparent);

    QPainter imagePainter(&m_cachedImage);
    imagePainter.setRenderHint(QPainter::Antialiasing);
    imagePainter.setRenderHint(QPainter::SmoothPixmapTransform);
    m_renderer->render(&imagePainter);
}

// ---------------------------------------------------------------------------
// animateTo — convenience wrapper around QPropertyAnimation
// ---------------------------------------------------------------------------

void SvgIcon::animateTo(const QVariantMap &targetOptions, int durationMs,
                        QEasingCurve::Type easing) {
    // Map option keys to their Q_PROPERTY names (they match, but be explicit)
    static const QList<QByteArray> animatableProps = {
        "color", "background", "opacity", "scale",
        "border_color", "border_width", "size"
    };

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    for (const QByteArray &prop : animatableProps) {
        if (!targetOptions.contains(prop))
            continue;

        QVariant target = targetOptions.value(prop);
        if (!target.isValid())
            continue;

        QPropertyAnimation *anim = new QPropertyAnimation(this, prop, group);
        anim->setDuration(durationMs);
        anim->setEndValue(target);
        anim->setEasingCurve(easing);
        group->addAnimation(anim);
    }

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
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    painter.scale(1.0 / m_devicePixelRatio, 1.0 / m_devicePixelRatio);

    const QRect rect = m_cachedImage.rect();

    // 1. Background fill
    painter.fillRect(rect, m_background);

    // 2. SVG content — either full render or single sprite element
    QImage coloredImage = m_cachedImage;

    if (!m_elementId.isEmpty()) {
        // Render only the requested sprite element
        m_renderer->render(&painter, m_elementId);
    } else if (!m_defaultColors && m_color != Qt::transparent) {
        // Apply tint color via CompositionMode_SourceIn (respects alpha)
        QPainter imagePainter(&coloredImage);
        imagePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        imagePainter.fillRect(rect, m_color);
    }
    // If m_defaultColors == true: draw as-is, no tinting

    // 3. Draw icon centered and scaled
    painter.setOpacity(m_opacity);
    const QSize imgSize = m_cachedImage.size() * m_scale;
    const QPoint center(
        (rect.width()  - imgSize.width())  / 2,
        (rect.height() - imgSize.height()) / 2
    );
    painter.drawImage(QRect(center, imgSize), coloredImage);

    // 4. Border — drawn last so it sits on top
    if (m_borderWidth > 0) {
        painter.setOpacity(1.0); // border always fully opaque
        QPen borderPen(m_borderColor, m_borderWidth);
        borderPen.setJoinStyle(Qt::MiterJoin);
        painter.setPen(borderPen);
        painter.setBrush(Qt::NoBrush);
        // inset by half border width so it doesn't clip at edges
        const qreal half = m_borderWidth / 2.0;
        painter.drawRect(QRectF(rect).adjusted(half, half, -half, -half));
    }
}
