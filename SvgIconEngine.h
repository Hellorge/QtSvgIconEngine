#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QString>
#include <QIcon>
#include <QPixmap>
#include <QVariantMap>

class SvgIconEngine
{
public:
    explicit SvgIconEngine(const QString &iconPath);
    ~SvgIconEngine();

    QIcon getIcon(const QString &style, const QString &iconName, const QVariantMap &colors = QVariantMap());

private:
    QPixmap createPixmap(const QString &filePath, const QVariantMap &colors);

    QString iconPath;
};

#endif // SVGICONENGINE_H
