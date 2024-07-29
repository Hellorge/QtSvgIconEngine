#ifndef SVGICON_H
#define SVGICON_H

#include <QSvgWidget>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QPropertyAnimation>
#include <QImage>

class SvgIcon : public QSvgWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal imageScale READ imageScale WRITE setImageScale)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

public:
    SvgIcon(QSvgRenderer *renderer, QVariantMap &options, QWidget *parent = nullptr);
    ~SvgIcon();

    QColor color() const;
    void setColor(const QColor &color);

    qreal opacity() const;
    void setOpacity(qreal opacity);

    qreal imageScale() const;
    void setImageScale(qreal scale);

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    void loadSvg(const QString &filePath);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSvgRenderer *m_renderer;
    QColor m_color;
    qreal m_opacity;
    qreal m_imageScale;
    QImage m_cachedImage;
    bool default_colors;

    void updateCachedImage();
    void setOptions(const QVariantMap &options);
};

#endif // SVGICON_H
