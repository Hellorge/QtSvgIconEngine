#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QPen>
#include <QString>
#include <QVariantMap>
#include <QCache>
#include <QMutex>
#include <QLoggingCategory>

enum class CachePolicy {
    LRU,  // up to 100 QSvgRenderer instances cached (default)
    ALL,  // cache everything
    NONE  // no caching
};

Q_DECLARE_LOGGING_CATEGORY(lcSvgIconEngine)

class SvgIconEngine : public QObject {
    Q_OBJECT

public:
    explicit SvgIconEngine(const QString &filePath,
                           const QVariantMap &options = QVariantMap());
    ~SvgIconEngine();

    // Get a full icon from an SVG file
    SvgIcon* getIcon(const QString &style, const QString &iconName);
    SvgIcon* getIcon(const QString &style, const QString &iconName, QVariantMap &options);

    // Get a single element out of a sprite SVG
    SvgIcon* getIconFromSprite(const QString &style, const QString &iconName,
                               const QString &elementId);
    SvgIcon* getIconFromSprite(const QString &style, const QString &iconName,
                               const QString &elementId, QVariantMap &options);

    void setDefaults(const QVariantMap &options);
    void clearCache();
    void setCachePolicy(CachePolicy policy);

private:
    QString iconPath;
    QVariantMap defOptions;
    QCache<QString, QSvgRenderer> rendererCache; // owns all QSvgRenderer pointers
    CachePolicy cachePolicy;
    QMutex cacheMutex;

    QVariantMap buildOptions(const QSvgRenderer *renderer, QVariantMap &options);
    const QString getFilePath(const QString &style, const QString &iconName);
    QSvgRenderer* getRenderer(const QString &filePath);
    QIcon drawNullIcon();
    void logError(const QString &message);
};

#endif // SVGICONENGINE_H
