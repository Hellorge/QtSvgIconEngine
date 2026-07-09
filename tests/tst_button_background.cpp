#include <QApplication>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include "SvgIconEngine.h"
#include "SvgIconButton.h"

int main(int argc,char**argv){
    QApplication app(argc,argv);
    SvgIconEngine e(argv[1]);
    QVariantMap o; o["size"]=QSize(32,32); o["color"]=QColor("#ffffff");

    auto *btn = new SvgIconButton(e.getIcon("regular/heart", o), "Label");
    btn->setButtonStyle(SvgIconButton::Flat);   // transparent chrome
    btn->resize(btn->sizeHint());
    btn->show(); app.processEvents();

    // Paint the button over a magenta canvas. If the icon still draws its
    // window brush, the area behind the heart will NOT be magenta.
    const QColor canvas("#ff00ff");
    QImage im(btn->size(), QImage::Format_ARGB32_Premultiplied);
    im.fill(canvas);
    { QPainter p(&im); btn->render(&p, QPoint(), QRegion(), QWidget::DrawChildren); }

    // sample the icon's top-left corner region (inside the icon rect, outside the glyph)
    const QColor corner = im.pixelColor(3, 3);
    const QColor windowBrush = btn->palette().color(QPalette::Window);
    int nonCanvas = 0;
    for (int y=0;y<im.height();++y) for (int x=0;x<im.width();++x)
        if (im.pixelColor(x,y) == windowBrush) ++nonCanvas;

    qInfo().noquote() << "  canvas colour      :" << canvas.name();
    qInfo().noquote() << "  window brush       :" << windowBrush.name();
    qInfo().noquote() << "  pixel behind icon  :" << corner.name();
    qInfo().noquote() << "  pixels painted with window brush:" << nonCanvas;
    

    const bool clean = (nonCanvas == 0);
    qInfo().noquote() << (clean ? "  PASS  icon draws no window background"
                                : "  FAIL  icon still paints a solid background card");
    return clean ? 0 : 1;
}
