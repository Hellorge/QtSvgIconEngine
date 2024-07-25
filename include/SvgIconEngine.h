#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include "SvgIcon.h"
#include <QString>
#include <QVariantMap>

class SvgIconEngine
{
public:
    explicit SvgIconEngine(const QString &filePath);
    ~SvgIconEngine();

    SvgIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &options = QVariantMap());

private:
    QPixmap createPixmap(const QString &filePath, const QVariantMap &options);

    QString iconPath;
};

#endif // SVGICONENGINE_H
