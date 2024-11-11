// This file is part of QtSvgIconEngine, under GNU LGPL license, for more info see LICENSE.

#include "SvgIcon.h"

SvgIcon::SvgIcon(QSvgRenderer *renderer, QVariantMap &options, QWidget *parent)
    : QSvgWidget(parent), m_renderer(renderer) {
    setOptions(options);
    updateCachedImage();
    m_devicePixelRatio = 1.0;
}

SvgIcon::~SvgIcon() {
    delete m_renderer;
}

void SvgIcon::setOptions(const QVariantMap &options) {
    m_color = options.value("color").value<QColor>();
    m_background = options.value("background").value<QColor>();
    m_opacity = options.value("opacity").toReal();
    m_scale = options.value("scale").toReal();
    m_borderColor = options.value("border_color").value<QColor>();
    m_borderWidth = options.value("border_width").toReal();
    default_colors = options.value("default_colors").toBool();
    setFixedSize(options.value("size").toSize());
}

const QVariantMap SvgIcon::getOptions() const {
	QVariantMap map;

	map.insert("color", color());
	map.insert("background", background());
	map.insert("opacity", opacity());
	map.insert("scale", scale());
	map.insert("border_color", borderColor());
	map.insert("border_width", borderWidth());
	map.insert("default_colors", default_colors);
	map.insert("size", size());

	return map;
}

QColor SvgIcon::color() const {
    return m_color;
}

void SvgIcon::setColor(const QColor &color) {
    if (m_color != color) {
        m_color = color;
        update();
    }
}

QColor SvgIcon::background() const {
    return m_background;
}

void SvgIcon::setBackground(const QColor &background) {
    if (m_background != background) {
        m_background = background;
        update();
    }
}

qreal SvgIcon::opacity() const {
    return m_opacity;
}

void SvgIcon::setOpacity(const qreal opacity) {
    if (m_opacity != opacity) {
        m_opacity = opacity;
        update();
    }
}

// QSize SvgIcon::size() const {
//     return size();
// }

void SvgIcon::setSize(const QSize &size) {
    if (size != this->size()) {
        setFixedSize(size);
        updateCachedImage();
        update();
    }
}

qreal SvgIcon::scale() const {
    return m_scale;
}

void SvgIcon::setScale(const qreal scale) {
    if (m_scale != scale) {
        m_scale = scale;
        update();
    }
}

QColor SvgIcon::borderColor() const {
    return m_borderColor;
}

void SvgIcon::setBorderColor(const QColor &borderColor) {
    if (m_borderColor != borderColor) {
        m_borderColor = borderColor;
        update();
    }
}

qreal SvgIcon::borderWidth() const {
    return m_borderWidth;
}

void SvgIcon::setBorderWidth(const qreal borderWidth) {
    if (m_borderWidth != borderWidth) {
        m_borderWidth = borderWidth;
        update();
    }
}

// void SvgIcon::loadSvg(const QString &filePath) {
//     delete m_renderer;
//     m_renderer = new QSvgRenderer(filePath, this);
//     updateCachedImage();
//     update();
// }

void SvgIcon::updateCachedImage() {
    if (m_renderer->isValid()) {
        m_cachedImage = QImage(size(), QImage::Format_ARGB32_Premultiplied);
        m_cachedImage.fill(Qt::transparent);
        QPainter imagePainter(&m_cachedImage);
        imagePainter.setRenderHint(QPainter::Antialiasing);
        imagePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        m_renderer->render(&imagePainter);
    }
}

void SvgIcon::setState(State state) {
    if (m_state != state) {
        m_state = state;
        updateStateStyle();
    }
}

void SvgIcon::updateStateStyle() {
    switch (m_state) {
        case Disabled:
            setOpacity(0.5);
            break;
        case Active:
            setColor(Qt::blue);
            break;
        case Selected:
            setBackground(Qt::lightGray);
            break;
        default:
            setOpacity(1.0);
            setColor(m_color);
            setBackground(Qt::transparent);
    }
    update();
}

void SvgIcon::setDevicePixelRatio(qreal dpr) {
    if (m_devicePixelRatio != dpr) {
        m_devicePixelRatio = dpr;
        updateScaledSize();
    }
}

void SvgIcon::updateScaledSize() {
    QSize scaledSize = size() * m_devicePixelRatio;
    m_cachedImage = QImage(scaledSize, QImage::Format_ARGB32_Premultiplied);
    m_cachedImage.setDevicePixelRatio(m_devicePixelRatio);
    updateCachedImage();
    update();
}

void SvgIcon::setElementId(QString elementId) {
	if (m_elementId != elementId) {
		m_elementId = elementId;
		update();
	}
}

void SvgIcon::paintEvent(QPaintEvent *event) {
    if (m_cachedImage.isNull()) {
        return;
    }

    QPainter painter(this);

    // QStyleOptionButton option;
	// option.initFrom(this);
	// option.state |= QStyle::State_Sunken;
	// option.state |= QStyle::State_On;
	// option.state |= QStyle::State_MouseOver;

	// style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    painter.scale(1.0 / m_devicePixelRatio, 1.0 / m_devicePixelRatio);

    const QRect rect = m_cachedImage.rect();
    painter.fillRect(rect, m_background);

    if (!m_elementId.isEmpty()) {
        m_renderer->render(&painter, m_elementId);
    }

    QImage coloredImage = m_cachedImage;
    if (m_color!= Qt::transparent) {
        QPainter imagePainter(&coloredImage);
        imagePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        imagePainter.fillRect(rect, m_color);
    }

    painter.setOpacity(m_opacity);

    const QSize imgSize = m_cachedImage.size() * m_scale;
    const QPoint center((width() - imgSize.width()) / 2, (height() - imgSize.height()) / 2);
    painter.drawImage(QRect(center, imgSize), coloredImage);

}
