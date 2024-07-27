#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QPen>
#include <QString>
#include <QPalette>
#include <QVariantMap>
#include <QCache>
#include <QtConcurrent>
#include <QMutex>

enum class CachePolicy {
    LRU, // Least Recently Used
    ALL, // First In First Out
    NONE // No caching
};

class SvgIconEngine : public QObject {
   Q_OBJECT

public:
    explicit SvgIconEngine(const QString &filePath, const QVariantMap &options = QVariantMap());
    ~SvgIconEngine();

    QIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &options = QVariantMap());
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

	QPixmap getPixmap(const QString &filePath, const QVariantMap &options);
    QSvgRenderer* getRenderer(const QString &filePath);
    QPixmap drawNullIcon();
    QString getCacheDirectory();
    void loadIconAsync(const QString &filePath);
    void logError(const QString &message);
    // void loadCacheFromDisk();

private slots:
    // void saveCacheToDisk();
};

#endif // SVGICONENGINE_H
