#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QPen>
#include <QString>
#include <QVariantMap>
#include <QCache>
#include <QMutex>
#include <QSharedPointer>
#include <QIcon>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcSvgIconEngine)

class SvgIconEngine : public QObject {
    Q_OBJECT

public:
    // `root` is the icon bank's base directory — a filesystem path or a qrc
    // prefix such as ":/icons". Every icon is addressed by a path relative to
    // it, or by an absolute/qrc path that ignores it. See resolvePath().
    explicit SvgIconEngine(const QString &root,
                           const QVariantMap &options = QVariantMap());
    ~SvgIconEngine();

    // Icons are addressed by path, the way an <img src> is:
    //
    //   "regular/heart"      -> <root>/regular/heart.svg   (root-relative)
    //   ":/other/x.svg"      -> verbatim                   (qrc, outside the bank)
    //   "/abs/path/x.svg"    -> verbatim                   (filesystem)
    //
    // A ".svg" suffix is optional. Inside a per-state option ("hover_icon"), a
    // bare name with no slash resolves as a sibling of the base icon, so
    // hover_icon="heart" means "the heart next to me" and hover_icon="solid/heart"
    // reaches across the bank.

    // An animatable widget.
    SvgIcon* getIcon(const QString &path);
    SvgIcon* getIcon(const QString &path, QVariantMap &options);

    // The same artwork as a QIcon, usable anywhere Qt takes one (QAction,
    // QPushButton::setIcon, QToolBar, item views).
    // QIcon cannot animate and has no pressed state — use getIcon() for those.
    QIcon icon(const QString &path);
    QIcon icon(const QString &path, QVariantMap &options);

    // A single element out of a sprite SVG, as a widget or as a QIcon.
    SvgIcon* getIconFromSprite(const QString &path, const QString &elementId);
    SvgIcon* getIconFromSprite(const QString &path, const QString &elementId,
                               QVariantMap &options);
    QIcon iconFromSprite(const QString &path, const QString &elementId);
    QIcon iconFromSprite(const QString &path, const QString &elementId,
                         QVariantMap &options);

    // Exposed because per-state icons and callers both need the same rules.
    QString resolvePath(const QString &path, const QString &baseDir = QString()) const;

    // Length of the longest subpath of an icon, or 0 if it has no stroke.
    // Measured once and memoised; see SvgStroke::measureStrokeLength.
    qreal strokeLength(const QString &path);

    void setDefaults(const QVariantMap &options);
    void clearCache();

    // Parsing an SVG is the expensive part, so renderers are kept in an LRU
    // cache. `renderers` is the number held; 0 disables caching entirely.
    // Default 100. (This replaces the old NONE/LRU/ALL policy enum — those were
    // three names for one number.)
    void setCacheLimit(int renderers);

private:
    QString iconPath;
    QVariantMap defOptions;

    // Renderers are reference-counted, not cache-owned. A QCache deletes evicted
    // values, which would dangle any SvgIcon still holding the renderer. Sharing
    // ownership means eviction only drops the cache's reference; the renderer
    // dies when the last SvgIcon using it does.
    QCache<QString, QSharedPointer<QSvgRenderer>> rendererCache;
    int cacheLimit = 100;
    QMutex cacheMutex;

    // Stroke effects need the raw bytes and a measured path length. Both are
    // memoised: measuring bisects over ~18 renders and is far too slow to redo.
    QHash<QString, QByteArray> sourceCache;
    QHash<QString, qreal> strokeLengthCache;
    QByteArray sourceFor(const QString &resolvedPath);
    // Takes an already-resolved path. The public strokeLength() resolves first;
    // calling that from an internal site would resolve twice.
    qreal strokeLengthForResolved(const QString &resolvedPath);
    void attachStrokeSource(SvgIcon *icon, const QString &resolvedPath,
                            const QVariantMap &options);

    QVariantMap buildOptions(const QSvgRenderer *renderer, QVariantMap &options);
    static QString baseDirOf(const QString &resolvedPath);
    QSharedPointer<QSvgRenderer> getRenderer(const QString &filePath);
    QHash<int, QSharedPointer<QSvgRenderer>> loadStateRenderers(const QString &baseDir,
                                                                const QVariantMap &options);
    void attachStateIcons(SvgIcon *icon, const QString &baseDir, const QVariantMap &options);
    void initDefaults();
    void logError(const QString &message);
};

#endif // SVGICONENGINE_H
