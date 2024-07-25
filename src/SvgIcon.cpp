#include "SvgIcon.h"

SvgIcon::SvgIcon(const QPixmap &pixmap)
    : QObject(nullptr), QIcon(pixmap), m_pixmap(pixmap), m_currentColor(Qt::transparent), m_currentRotation(0)
{
}

SvgIcon::~SvgIcon()
{
}

void SvgIcon::animateColorChange(const QColor &startColor, const QColor &endColor, int duration)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "color");
    animation->setDuration(duration);
    animation->setStartValue(startColor);
    animation->setEndValue(endColor);
    connect(animation, &QPropertyAnimation::valueChanged, [this](const QVariant &value){
        applyColorChange(value.value<QColor>());
        updateIcon();
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SvgIcon::rotate(int angle, int duration)
{
    QPropertyAnimation *animation = new QPropertyAnimation(this, "rotation");
    animation->setDuration(duration);
    animation->setStartValue(m_currentRotation);
    animation->setEndValue(angle);
    connect(animation, &QPropertyAnimation::valueChanged, [this](const QVariant &value){
        applyRotation(value.toInt());
        updateIcon();
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

QColor SvgIcon::color() const
{
    return m_currentColor;
}

void SvgIcon::setColor(const QColor &color)
{
    if (m_currentColor != color) {
        m_currentColor = color;
        emit colorChanged();
        updateIcon();
    }
}

int SvgIcon::rotation() const
{
    return m_currentRotation;
}

void SvgIcon::setRotation(int rotation)
{
    if (m_currentRotation != rotation) {
        m_currentRotation = rotation;
        emit rotationChanged();
        updateIcon();
    }
}

void SvgIcon::updateIcon()
{
    QPixmap newPixmap = m_pixmap;
    QPainter painter(&newPixmap);

    // Apply color change
    if (m_currentColor != Qt::transparent) {
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(newPixmap.rect(), m_currentColor);
    }

    // Apply rotation
    if (m_currentRotation != 0) {
        QTransform transform;
        transform.rotate(m_currentRotation);
        newPixmap = newPixmap.transformed(transform);
    }

    painter.end();
    QIcon::addPixmap(newPixmap);
}

void SvgIcon::applyColorChange(const QColor &color)
{
    m_currentColor = color;
}

void SvgIcon::applyRotation(int angle)
{
    m_currentRotation = angle;
}
