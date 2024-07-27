#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QPen>
#include <QString>
#include <QPalette>
#include <QVariantMap>
#include <QCache>
#include <QtConcurrent>

class SvgIconEngine
{
public:
    explicit SvgIconEngine(const QString &filePath, const QVariantMap &options = QVariantMap());
    ~SvgIconEngine();

    QIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &options = QVariantMap());
	void setDefaults(const QVariantMap &options);
    void clearCache();

private:
	// static QPen pen;
	QString iconPath;
	QVariantMap defOptions;
	QCache<QString, QPixmap> pixmapCache;

	QPixmap getPixmap(const QString &filePath);
    QPixmap createPixmap(const QString &filePath);
    QPixmap applyOptions(QPixmap pixmap, const QVariantMap &options);
    QPixmap drawNullIcon();
    void loadIconAsync(const QString &filePath);
    void logError(const QString &message);

};

#endif // SVGICONENGINE_H
