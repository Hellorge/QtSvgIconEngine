#ifndef SVGICON_H
#define SVGICON_H

#include <QSvgWidget>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QPropertyAnimation>
#include <QImage>
#include <QStyle>
#include <QStyleOptionButton>

class SvgIcon : public QSvgWidget {
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(QColor background READ background WRITE setBackground)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QSize size READ size WRITE setSize)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(QColor border_color READ borderColor WRITE setBorderColor)
    Q_PROPERTY(qreal border_width READ borderWidth WRITE setBorderWidth)

public:
	enum WidgetType { Button, Label };
	enum State { Normal, Disabled, Active, Selected };

	SvgIcon(QSvgRenderer *renderer, QVariantMap &options, QWidget *parent = nullptr);
    ~SvgIcon();

    QColor color() const;
    void setColor(const QColor &color);

    QColor background() const;
    void setBackground(const QColor &background);

    qreal opacity() const;
    void setOpacity(const qreal opacity);

    // QSize size() const;
    void setSize(const QSize &size);

    qreal scale() const;
    void setScale(const qreal scale);

    QColor borderColor() const;
    void setBorderColor(const QColor &borderColor);

    qreal borderWidth() const;
    void setBorderWidth(const qreal borderWidth);

    void setState(State state);
    void setDevicePixelRatio(qreal dpr);
    void setElementId(QString elementId);

    // void loadSvg(const QString &filePath);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QSvgRenderer *m_renderer;
    QImage m_cachedImage;
    QIcon m_icon;

    QColor m_color;
    QColor m_background;
    qreal m_opacity;
    qreal m_scale;
    QColor m_borderColor;
    qreal m_borderWidth;
    bool default_colors;
    QString m_elementId;

    State m_state;
    qreal m_devicePixelRatio;

    void updateScaledSize();
    void updateStateStyle();
    void updateCachedImage();
    void setOptions(const QVariantMap &options);
    const QVariantMap getOptions() const;
};

#endif // SVGICON_H
