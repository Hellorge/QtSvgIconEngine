// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIconEngine.h"
#include "SvgQIconEngine.h"
#include "SvgStroke.h"
#include <QFile>
#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QDir>

Q_LOGGING_CATEGORY(lcSvgIconEngine, "svgiconengine")

SvgIconEngine::SvgIconEngine(const QString &root, const QVariantMap &options)
    : iconPath(root) {
    initDefaults();
    setDefaults(options);
    rendererCache.setMaxCost(cacheLimit);
}

SvgIconEngine::~SvgIconEngine() {
    // The cache holds QSharedPointer references. Clearing it drops those
    // references; renderers still held by live SvgIcons stay alive.
}

void SvgIconEngine::initDefaults() {
    defOptions.insert("background",     QColor(Qt::transparent));
    defOptions.insert("default_colors", false);
    defOptions.insert("opacity",        1.0);
    defOptions.insert("border_width",   0.0);
    defOptions.insert("scale",          1.0);

    // "color" and "border_color" are deliberately absent: an icon with no
    // explicit colour inherits QPalette::Text at paint time and keeps following
    // the palette across theme changes. Baking the colour in here would freeze
    // it at engine-construction time.
    //
    // "size" has no sensible universal default — falls back to renderer->defaultSize()
}

void SvgIconEngine::setDefaults(const QVariantMap &options) {
    // Merge into the existing defaults rather than resetting them, so that
    // successive calls accumulate instead of silently dropping earlier values.
    for (auto it = options.constBegin(); it != options.constEnd(); ++it) {
        const QString &property = it.key();
        if (it.value().isValid()) {
            defOptions.insert(property, it.value());
        } else {
            qCWarning(lcSvgIconEngine) << "SvgIconEngine: invalid value for option" << property;
        }
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

SvgIcon* SvgIconEngine::getIcon(const QString &path) {
    QVariantMap options;
    return getIcon(path, options);
}

SvgIcon* SvgIconEngine::getIcon(const QString &path, QVariantMap &options) {
    const QString filePath = resolvePath(path);
    QSharedPointer<QSvgRenderer> renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return nullptr;
    }

    options = buildOptions(renderer.data(), options);
    SvgIcon *icon = new SvgIcon(renderer, options);
    attachStateIcons(icon, baseDirOf(filePath), options);
    attachStrokeSource(icon, filePath, options);
    return icon;
}

// Only icons that actually ask for a stroke effect pay for the source bytes and
// the (expensive) length measurement.
void SvgIconEngine::attachStrokeSource(SvgIcon *icon, const QString &resolvedPath,
                                       const QVariantMap &options) {
    static const char *kKeys[] = {
        "stroke_progress", "hover_stroke_progress", "pressed_stroke_progress",
        "selected_stroke_progress", "disabled_stroke_progress",
        "dash_pattern", "stroke_effects"
    };
    bool wanted = false;
    for (const char *k : kKeys)
        if (options.contains(QString::fromLatin1(k))) { wanted = true; break; }
    if (!wanted)
        return;

    const QByteArray svg = sourceFor(resolvedPath);
    if (svg.isEmpty())
        return;

    const qreal len = strokeLengthForResolved(resolvedPath);
    if (qFuzzyIsNull(len))
        qCWarning(lcSvgIconEngine) << resolvedPath
            << "has no stroke; stroke effects will be a no-op";

    icon->setStrokeSource(svg, len);

    if (options.contains(QStringLiteral("dash_pattern")))
        icon->setDashPattern(options.value(QStringLiteral("dash_pattern")).toReal());
    if (options.contains(QStringLiteral("dash_offset")))
        icon->setDashOffset(options.value(QStringLiteral("dash_offset")).toReal());
}

QByteArray SvgIconEngine::sourceFor(const QString &resolvedPath) {
    auto it = sourceCache.constFind(resolvedPath);
    if (it != sourceCache.constEnd())
        return *it;

    QFile f(resolvedPath);
    if (!f.open(QIODevice::ReadOnly)) {
        logError(QString("Failed to read SVG source: '%1'").arg(resolvedPath));
        return {};
    }
    const QByteArray bytes = f.readAll();
    sourceCache.insert(resolvedPath, bytes);
    return bytes;
}

qreal SvgIconEngine::strokeLength(const QString &path) {
    return strokeLengthForResolved(resolvePath(path));
}

qreal SvgIconEngine::strokeLengthForResolved(const QString &resolvedPath) {
    auto it = strokeLengthCache.constFind(resolvedPath);
    if (it != strokeLengthCache.constEnd())
        return *it;

    const qreal len = SvgStroke::measureStrokeLength(sourceFor(resolvedPath));
    strokeLengthCache.insert(resolvedPath, len);
    return len;
}

// Per-state artwork is requested by path ("hover_icon": "solid/heart"), or by a
// bare name resolved as a sibling of the base icon. A missing file warns and
// leaves the state showing the base artwork rather than failing the whole icon.
QHash<int, QSharedPointer<QSvgRenderer>>
SvgIconEngine::loadStateRenderers(const QString &baseDir, const QVariantMap &options) {
    static const struct { const char *key; SvgIcon::State state; } kStateIcons[] = {
        { "hover_icon",    SvgIcon::Hovered  },
        { "pressed_icon",  SvgIcon::Pressed  },
        { "selected_icon", SvgIcon::Selected },
        { "disabled_icon", SvgIcon::Disabled },
    };

    QHash<int, QSharedPointer<QSvgRenderer>> out;
    for (const auto &entry : kStateIcons) {
        const QString path = options.value(QString::fromLatin1(entry.key)).toString();
        if (path.isEmpty())
            continue;

        QSharedPointer<QSvgRenderer> r = getRenderer(resolvePath(path, baseDir));
        if (!r) {
            qCWarning(lcSvgIconEngine) << "State icon" << entry.key << "->" << path
                                       << "could not be loaded; using base artwork";
            continue;
        }
        out.insert(entry.state, r);
    }
    return out;
}

void SvgIconEngine::attachStateIcons(SvgIcon *icon, const QString &baseDir,
                                     const QVariantMap &options) {
    const auto renderers = loadStateRenderers(baseDir, options);
    for (auto it = renderers.constBegin(); it != renderers.constEnd(); ++it)
        icon->setStateRenderer(static_cast<SvgIcon::State>(it.key()), it.value());
}

// ---------------------------------------------------------------------------
// QIcon production
// ---------------------------------------------------------------------------

QIcon SvgIconEngine::icon(const QString &path) {
    QVariantMap options;
    return icon(path, options);
}

QIcon SvgIconEngine::icon(const QString &path, QVariantMap &options) {
    const QString filePath = resolvePath(path);
    QSharedPointer<QSvgRenderer> renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return QIcon();
    }

    options = buildOptions(renderer.data(), options);
    return QIcon(new SvgQIconEngine(renderer, loadStateRenderers(baseDirOf(filePath), options),
                                    options, filePath, QString()));
}

// ---------------------------------------------------------------------------
// Sprites
// ---------------------------------------------------------------------------

SvgIcon* SvgIconEngine::getIconFromSprite(const QString &path, const QString &elementId) {
    QVariantMap options;
    return getIconFromSprite(path, elementId, options);
}

SvgIcon* SvgIconEngine::getIconFromSprite(const QString &path, const QString &elementId,
                                          QVariantMap &options) {
    const QString filePath = resolvePath(path);
    QSharedPointer<QSvgRenderer> renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return nullptr;
    }
    if (!renderer->elementExists(elementId)) {
        qCWarning(lcSvgIconEngine) << "No element" << elementId << "in sprite" << filePath;
        return nullptr;
    }

    options = buildOptions(renderer.data(), options);

    SvgIcon *icon = new SvgIcon(renderer, options);
    icon->setElementId(elementId);
    attachStateIcons(icon, baseDirOf(filePath), options);
    return icon;
}

QIcon SvgIconEngine::iconFromSprite(const QString &path, const QString &elementId) {
    QVariantMap options;
    return iconFromSprite(path, elementId, options);
}

QIcon SvgIconEngine::iconFromSprite(const QString &path, const QString &elementId,
                                    QVariantMap &options) {
    const QString filePath = resolvePath(path);
    QSharedPointer<QSvgRenderer> renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return QIcon();
    }
    if (!renderer->elementExists(elementId)) {
        qCWarning(lcSvgIconEngine) << "No element" << elementId << "in sprite" << filePath;
        return QIcon();
    }

    options = buildOptions(renderer.data(), options);
    return QIcon(new SvgQIconEngine(renderer, loadStateRenderers(baseDirOf(filePath), options),
                                    options, filePath, elementId));
}

void SvgIconEngine::clearCache() {
    QMutexLocker locker(&cacheMutex);
    rendererCache.clear();
}

void SvgIconEngine::setCacheLimit(int renderers) {
    QMutexLocker locker(&cacheMutex);
    cacheLimit = qMax(0, renderers);
    if (cacheLimit == 0)
        rendererCache.clear();
    else
        rendererCache.setMaxCost(cacheLimit);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

// Addressing works like an <img src>:
//   ":/x.svg" or "/x.svg" -> verbatim (qrc / filesystem, outside the bank)
//   "regular/heart"       -> <root>/regular/heart.svg
//   "heart" with baseDir  -> <baseDir>/heart.svg   (sibling of the base icon)
// The ".svg" suffix is optional everywhere.
QString SvgIconEngine::resolvePath(const QString &path, const QString &baseDir) const {
    if (path.isEmpty())
        return path;

    QString p = path;
    if (!p.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive))
        p += QLatin1String(".svg");

    // Absolute or qrc: ignore the bank entirely.
    //
    // QDir::isAbsolutePath rather than a leading '/': on Windows an absolute path
    // is "C:/x" or "\\\\server\\share", neither of which starts with a slash. A
    // qrc path starts with ':' and is checked separately.
    if (p.startsWith(QLatin1Char(':')) || QDir::isAbsolutePath(p))
        return p;

    // A bare name inside a per-state option means "next to the base icon".
    if (!baseDir.isEmpty() && !p.contains(QLatin1Char('/')))
        return baseDir + QLatin1Char('/') + p;

    return iconPath + QLatin1Char('/') + p;
}

QString SvgIconEngine::baseDirOf(const QString &resolvedPath) {
    const int slash = resolvedPath.lastIndexOf(QLatin1Char('/'));
    return slash > 0 ? resolvedPath.left(slash) : QString();
}

QSharedPointer<QSvgRenderer> SvgIconEngine::getRenderer(const QString &filePath) {
    QMutexLocker locker(&cacheMutex);

    if (cacheLimit > 0) {
        if (QSharedPointer<QSvgRenderer> *cached = rendererCache.object(filePath)) {
            if (*cached && (*cached)->isValid())
                return *cached;
        }
    }

    QSharedPointer<QSvgRenderer> renderer(new QSvgRenderer(filePath));
    if (!renderer->isValid()) {
        logError(QString("Failed to load SVG: '%1'").arg(filePath));
        return {};
    }

    if (cacheLimit > 0) {
        // The cache owns a *reference*, not the renderer. Eviction deletes this
        // QSharedPointer holder and drops one reference; any SvgIcon still using
        // the renderer keeps it alive.
        rendererCache.insert(filePath, new QSharedPointer<QSvgRenderer>(renderer));
    }
    // With caching off the renderer is owned solely by the returned
    // QSharedPointer, so it is freed with the last SvgIcon rather than leaked.

    return renderer;
}

QVariantMap SvgIconEngine::buildOptions(const QSvgRenderer *renderer, QVariantMap &options) {
    // Size fallback: caller option → engine default → renderer native size
    if (!options.contains("size") || !options.value("size").isValid()) {
        if (defOptions.contains("size") && defOptions.value("size").isValid()) {
            options.insert("size", defOptions.value("size"));
        } else {
            options.insert("size", renderer->defaultSize());
        }
    }

    // Fill remaining options from engine defaults if not provided by caller
    for (auto it = defOptions.constBegin(); it != defOptions.constEnd(); ++it) {
        const QString &key = it.key();
        if (!options.contains(key) || !options.value(key).isValid()) {
            options.insert(key, it.value());
        }
    }

    return options;
}

void SvgIconEngine::logError(const QString &message) {
    qCCritical(lcSvgIconEngine) << "SvgIconEngine Error:" << message;
}
