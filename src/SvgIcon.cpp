#include "SvgIcon.h"

SvgIcon::SvgIcon(QSvgRenderer *renderer, QVariantMap &options, QWidget *parent)
    : QSvgWidget(parent), m_renderer(renderer)  {
    // setMinimumSize(100, 100);
    // setMaximumSize(100, 100);

    setOptions(options);

    updateCachedImage();
}

SvgIcon::~SvgIcon() {
    delete m_renderer;
}

void SvgIcon::setOptions(const QVariantMap &options) {
    m_color = options.value("color").value<QColor>();
    m_opacity = options.value("opacity").toInt();
    m_imageScale = 1.0;
    setFixedSize(options.value("size").toSize());
}

QColor SvgIcon::color() const {
    return m_color;
}

void SvgIcon::setColor(const QColor &color) {
    if (color != m_color) {
        m_color = color;
        update();
    }
}

qreal SvgIcon::opacity() const {
    return m_opacity;
}

void SvgIcon::setOpacity(qreal opacity) {
    if (opacity != m_opacity) {
        m_opacity = opacity;
        update();
    }
}

QSize SvgIcon::iconSize() const {
    return size();
}

void SvgIcon::setIconSize(const QSize &size) {
    if (size != this->size()) {
        setFixedSize(size);
        updateCachedImage();
        update();
    }
}

qreal SvgIcon::imageScale() const {
    return m_imageScale;
}

void SvgIcon::setImageScale(qreal scale) {
    if (scale != m_imageScale) {
        m_imageScale = scale;
        update();
    }
}

void SvgIcon::loadSvg(const QString &filePath) {
    delete m_renderer;
    m_renderer = new QSvgRenderer(filePath, this);
    updateCachedImage();
    update();
}

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

void SvgIcon::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    if (!m_cachedImage.isNull()) {
        QImage coloredImage = m_cachedImage;
        QPainter imagePainter(&coloredImage);
        imagePainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        imagePainter.fillRect(coloredImage.rect(), m_color);
        imagePainter.end();

        painter.setOpacity(m_opacity);

        QSize imgSize = m_cachedImage.size() * m_imageScale;
        QPoint center((width() - imgSize.width()) / 2, (height() - imgSize.height()) / 2);
        painter.drawImage(QRect(center, imgSize), coloredImage);
    }
}
