#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include "SvgIconEngine.h"
#include "SvgIcon.h"

int main(int argc,char**argv){
    QApplication app(argc,argv);
    SvgIconEngine e(argv[1]);
    const qreal dpr = app.devicePixelRatio();

    QVariantMap o; o["size"]=QSize(32,32); o["color"]=QColor("#ffffff");
    SvgIcon *w = e.getIcon("regular/heart", o);

    // Render the WIDGET at the screen's dpr
    QImage wi(QSize(32,32)*dpr, QImage::Format_ARGB32_Premultiplied);
    wi.fill(Qt::transparent); wi.setDevicePixelRatio(dpr);
    { QPainter p(&wi); w->render(&p, QPoint(), QRegion(), QWidget::DrawChildren); }

    // Render the QICON at the same dpr (known-correct path)
    QVariantMap o2; o2["color"]=QColor("#ffffff");
    QIcon ic = e.icon("regular/heart", o2);
    QImage qi = ic.pixmap(QSize(32,32), dpr, QIcon::Normal, QIcon::Off).toImage();

    // mean absolute difference on the alpha channel
    long diff = 0; int n = 0;
    for (int y=0;y<qi.height() && y<wi.height();++y)
      for (int x=0;x<qi.width() && x<wi.width();++x) {
        diff += qAbs(qi.pixelColor(x,y).alpha() - wi.pixelColor(x,y).alpha()); ++n;
      }
    qInfo().noquote() << QString("  dpr=%1  widget %2x%3  qicon %4x%5  mean alpha diff %6")
        .arg(dpr).arg(wi.width()).arg(wi.height()).arg(qi.width()).arg(qi.height())
        .arg(n ? double(diff)/n : 0.0, 0, 'f', 2);
    const double mad = n ? double(diff)/n : 0.0;
    const bool crisp = mad < 1.0;
    qInfo().noquote() << (crisp ? "  PASS  widget rasterises at screen dpr (matches QIcon)"
                                : "  FAIL  widget is upscaled from a 1x bitmap");
    return crisp ? 0 : 1;
}
