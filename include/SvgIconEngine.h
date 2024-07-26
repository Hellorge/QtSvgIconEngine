#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QPen>
#include <QString>
#include <QPalette>
#include <QVariantMap>

class SvgIconEngine
{
public:
    explicit SvgIconEngine(const QString &filePath);
    ~SvgIconEngine();

    QIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &options = QVariantMap());

private:
	QPixmap getPixmap(const QString &filePath);
    QPixmap createPixmap(const QString &filePath);
    QPixmap applyOptions(QPixmap pixmap, const QVariantMap &options);
    QPixmap drawNullIcon();

    // static QPen pen;
    QString iconPath;
    const QColor defIconColor;
    QHash<QString, QPixmap> pixmapCache;
};

#endif // SVGICONENGINE_H
