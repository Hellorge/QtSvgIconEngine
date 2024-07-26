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

    SvgIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &options = QVariantMap());

private:
    QPixmap createPixmap(const QString &filePath, const QVariantMap &options);
    void drawNullIcon(QPixmap &pixmap);

    QPen pen;
    QString iconPath;
};

#endif // SVGICONENGINE_H
