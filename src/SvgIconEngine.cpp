// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIconEngine.h"
#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QDir>

Q_LOGGING_CATEGORY(lcSvgIconEngine, "svgiconengine")

SvgIconEngine::SvgIconEngine(const QString &filePath, const QVariantMap &options)
    : iconPath(filePath), cachePolicy(CachePolicy::LRU) {
    setDefaults(options);
    setCachePolicy(cachePolicy);
}

SvgIconEngine::~SvgIconEngine() {
    // rendererCache (QCache) owns and deletes all QSvgRenderer pointers.
    // SvgIcon instances must NOT delete renderers — see SvgIcon destructor.
}

void SvgIconEngine::setDefaults(const QVariantMap &options) {
    defOptions.insert("color",          QApplication::palette().text().color());
    defOptions.insert("background",     QColor(Qt::transparent));
    defOptions.insert("default_colors", false);
    defOptions.insert("opacity",        1.0);
    defOptions.insert("border_color",   QApplication::palette().text().color());
    defOptions.insert("border_width",   0.0);
    defOptions.insert("scale",          1.0);

    for (auto it = defOptions.constBegin(); it != defOptions.constEnd(); ++it) {
        const QString &property = it.key();
        if (options.contains(property) && options.value(property).isValid()) {
            defOptions.insert(property, options.value(property));
        } else if (options.contains(property)) {
            qCWarning(lcSvgIconEngine) << "SvgIconEngine: invalid value for option" << property;
        }
    }

    // size has no sensible universal default — falls back to renderer->defaultSize()
    if (options.contains("size") && options.value("size").isValid()) {
        defOptions.insert("size", options.value("size"));
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

SvgIcon* SvgIconEngine::getIcon(const QString &style, const QString &iconName) {
    QVariantMap options;
    return getIcon(style, iconName, options);
}

SvgIcon* SvgIconEngine::getIcon(const QString &style, const QString &iconName,
                                QVariantMap &options) {
    const QString filePath = getFilePath(style, iconName);
    QSvgRenderer *renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return nullptr;
    }

    options = buildOptions(renderer, options);
    return new SvgIcon(renderer, options);
}

SvgIcon* SvgIconEngine::getIconFromSprite(const QString &style, const QString &iconName,
                                          const QString &elementId) {
    QVariantMap options;
    return getIconFromSprite(style, iconName, elementId, options);
}

SvgIcon* SvgIconEngine::getIconFromSprite(const QString &style, const QString &iconName,
                                          const QString &elementId, QVariantMap &options) {
    const QString filePath = getFilePath(style, iconName);
    QSvgRenderer *renderer = getRenderer(filePath);

    if (!renderer) {
        qCWarning(lcSvgIconEngine) << "Failed to get renderer for" << filePath;
        return nullptr;
    }

    options["viewBox"] = renderer->boundsOnElement(elementId);
    options = buildOptions(renderer, options);

    SvgIcon *icon = new SvgIcon(renderer, options);
    icon->setElementId(elementId);
    return icon;
}

void SvgIconEngine::clearCache() {
    rendererCache.clear();
}

void SvgIconEngine::setCachePolicy(CachePolicy policy) {
    cachePolicy = policy;
    rendererCache.setMaxCost(policy == CachePolicy::ALL ? INT_MAX : 100);
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

const QString SvgIconEngine::getFilePath(const QString &style, const QString &iconName) {
    return QString("%1/%2/%3.svg").arg(iconPath, style, iconName);
}

QSvgRenderer* SvgIconEngine::getRenderer(const QString &filePath) {
    if (cachePolicy != CachePolicy::NONE) {
        QSvgRenderer *cached = rendererCache.object(filePath);
        if (cached && cached->isValid())
            return cached;
    }

    QSvgRenderer *renderer = new QSvgRenderer(filePath);
    if (!renderer->isValid()) {
        logError(QString("Failed to load SVG: '%1'").arg(filePath));
        delete renderer;
        return nullptr;
    }

    if (cachePolicy != CachePolicy::NONE) {
        // QCache takes ownership — do not delete renderer after this point.
        // SvgIcon instances receive this pointer but must NOT delete it.
        rendererCache.insert(filePath, renderer);
    }

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

QIcon SvgIconEngine::drawNullIcon() {
    QPixmap pixmap(100, 100);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    QPen pen(defOptions.value("color").value<QColor>(), 8);
    painter.setPen(pen);
    painter.drawRect(1, 1, 98, 98);
    painter.drawLine(0, 0, 99, 99);
    painter.drawLine(0, 99, 99, 0);
    painter.end();

    return QIcon(pixmap);
}

void SvgIconEngine::logError(const QString &message) {
    qCCritical(lcSvgIconEngine) << "SvgIconEngine Error:" << message;
}
