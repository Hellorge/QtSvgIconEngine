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
    LRU, // Least Recently Used
    ALL, // First In First Out
    NONE // No caching
};

Q_DECLARE_LOGGING_CATEGORY(lcSvgIconEngine)

class SvgIconEngine : public QObject {
   Q_OBJECT

public:
    explicit SvgIconEngine(const QString &filePath, const QVariantMap &options = QVariantMap());
    ~SvgIconEngine();

    SvgIcon* getIcon(const QString &style, const QString &iconName);
    SvgIcon* getIcon(const QString &style, const QString &iconName, QVariantMap &options);
	void setDefaults(const QVariantMap &options);
    void clearCache();
    void setCachePolicy(CachePolicy policy);

private:
	// static QPen pen;
	QString iconPath;
	QVariantMap defOptions;
	QCache<QString, QSvgRenderer> rendererCache;
	CachePolicy cachePolicy;
	QMutex cacheMutex;
	QList<QString> iconProperties;

    QVariantMap getOptions(const QSvgRenderer *renderer, QVariantMap &options);
	QSvgRenderer* getRenderer(const QString &filePath);
    QIcon drawNullIcon();
    QString getCacheDirectory();
    void loadIconAsync(const QString &filePath);
    void logError(const QString &message);
    // void loadCacheFromDisk();

private slots:
    // void saveCacheToDisk();
};

#endif // SVGICONENGINE_H
