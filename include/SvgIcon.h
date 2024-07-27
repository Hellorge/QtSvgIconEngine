#ifndef SVGICON_H
#define SVGICON_H

#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QObject>
#include <QPainter>
#include <QSvgRenderer>
#include <QPropertyAnimation>

class SvgIcon : public QObject, public QIcon
{
    Q_OBJECT
    // Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    // Q_PROPERTY(int rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    SvgIcon(const QPixmap &pixmap);
    ~SvgIcon();
};

#endif // SVGICON_H
