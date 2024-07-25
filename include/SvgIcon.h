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
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int rotation READ rotation WRITE setRotation NOTIFY rotationChanged)

public:
    SvgIcon(const QPixmap &pixmap);
    ~SvgIcon();

    void animateColorChange(const QColor &startColor, const QColor &endColor, int duration);
    void rotate(int angle, int duration);

    QColor color() const;
    void setColor(const QColor &color);

    int rotation() const;
    void setRotation(int rotation);

signals:
    void colorChanged();
    void rotationChanged();

private:
    QPixmap m_pixmap;
    QColor m_currentColor;
    int m_currentRotation;

    void updateIcon();
    void applyColorChange(const QColor &color);
    void applyRotation(int angle);
};

#endif // SVGICON_H
