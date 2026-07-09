// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgQIconEngine.h"
#include "SvgIconPainter.h"

#include <QPainter>
#include <QPixmapCache>

SvgQIconEngine::SvgQIconEngine(const QSharedPointer<QSvgRenderer> &base,
                               const QHash<int, QSharedPointer<QSvgRenderer>> &stateRenderers,
                               const QVariantMap &options,
                               const QString &name,
                               const QString &elementId)
    : m_base(base), m_stateRenderers(stateRenderers), m_options(options),
      m_name(name), m_elementId(elementId) {}

SvgIcon::State SvgQIconEngine::toIconState(QIcon::Mode mode) {
    switch (mode) {
        case QIcon::Active:   return SvgIcon::Hovered;
        case QIcon::Selected: return SvgIcon::Selected;
        case QIcon::Disabled: return SvgIcon::Disabled;
        case QIcon::Normal:
        default:              return SvgIcon::Normal;
    }
}

QSvgRenderer *SvgQIconEngine::rendererFor(SvgIcon::State state) const {
    const QSharedPointer<QSvgRenderer> r = m_stateRenderers.value(state, m_base);
    return r ? r.data() : nullptr;
}

bool SvgQIconEngine::isNull() {
    return !m_base || !m_base->isValid();
}

void SvgQIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state) {
    Q_UNUSED(state)
    const SvgIcon::State s = toIconState(mode);
    QSvgRenderer *r = rendererFor(s);
    if (!r)
        return;

    const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
    const QImage uncolored = SvgIconPainter::rasterize(r, rect.size(), dpr, m_elementId);
    SvgIconPainter::composite(painter, rect, uncolored,
                              SvgIcon::resolveOptions(m_options, s));
}

QPixmap SvgQIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) {
    return scaledPixmap(size, mode, state, 1.0);
}

QPixmap SvgQIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode,
                                     QIcon::State state, qreal scale) {
    Q_UNUSED(state)
    if (size.isEmpty() || scale <= 0.0)
        return QPixmap();

    const SvgIcon::State s = toIconState(mode);
    const QVariantMap resolved = SvgIcon::resolveOptions(m_options, s);

    // The colour is part of the key: two icons from the same file but different
    // tints (or a palette change) must not collide in the cache.
    const QString cacheKey = QStringLiteral("svgqie:%1#%7:%2x%3@%4:%5:%6")
        .arg(m_name)
        .arg(size.width()).arg(size.height())
        .arg(scale)
        .arg(int(mode))
        .arg(resolved.value(QStringLiteral("color")).value<QColor>().name(QColor::HexArgb))
        .arg(m_elementId);

    QPixmap cached;
    if (QPixmapCache::find(cacheKey, &cached))
        return cached;

    QSvgRenderer *r = rendererFor(s);
    if (!r)
        return QPixmap();

    QImage out(size * scale, QImage::Format_ARGB32_Premultiplied);
    out.setDevicePixelRatio(scale);
    out.fill(Qt::transparent);

    {
        QPainter painter(&out);
        const QImage uncolored = SvgIconPainter::rasterize(r, size, scale, m_elementId);
        SvgIconPainter::composite(&painter, QRect(QPoint(0, 0), size), uncolored, resolved);
    }

    QPixmap pm = QPixmap::fromImage(out);
    pm.setDevicePixelRatio(scale);
    QPixmapCache::insert(cacheKey, pm);
    return pm;
}

QIconEngine *SvgQIconEngine::clone() const {
    return new SvgQIconEngine(m_base, m_stateRenderers, m_options, m_name, m_elementId);
}

