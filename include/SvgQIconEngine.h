#ifndef SVGQICONENGINE_H
#define SVGQICONENGINE_H

#include <QIconEngine>
#include <QSharedPointer>
#include <QSvgRenderer>
#include <QVariantMap>
#include <QHash>

#include "SvgIcon.h"

// A real QIconEngine, so an SVG produced by SvgIconEngine can be handed to any
// Qt API that takes a QIcon — QAction, QPushButton::setIcon, QToolBar, item
// views — and still be tinted, per-state coloured, and given per-state artwork.
//
// QIcon has no notion of a pressed state, and cannot animate. Use the SvgIcon
// widget when you need either. QIcon::Mode maps onto SvgIcon::State as:
//
//     QIcon::Normal   -> SvgIcon::Normal
//     QIcon::Active   -> SvgIcon::Hovered
//     QIcon::Selected -> SvgIcon::Selected
//     QIcon::Disabled -> SvgIcon::Disabled
//
class SvgQIconEngine : public QIconEngine {
public:
    // elementId non-empty renders a single element of a sprite sheet.
    SvgQIconEngine(const QSharedPointer<QSvgRenderer> &base,
                   const QHash<int, QSharedPointer<QSvgRenderer>> &stateRenderers,
                   const QVariantMap &options,
                   const QString &name,
                   const QString &elementId = QString());

    void paint(QPainter *painter, const QRect &rect,
               QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state,
                         qreal scale) override;

    QIconEngine *clone() const override;
    QString key() const override { return QStringLiteral("SvgQIconEngine"); }
    QString iconName() override { return m_name; }
    bool isNull() override;

    static SvgIcon::State toIconState(QIcon::Mode mode);

private:
    QSvgRenderer *rendererFor(SvgIcon::State state) const;

    QSharedPointer<QSvgRenderer> m_base;
    QHash<int, QSharedPointer<QSvgRenderer>> m_stateRenderers;
    QVariantMap m_options;
    QString m_name;
    QString m_elementId;
};

#endif // SVGQICONENGINE_H
