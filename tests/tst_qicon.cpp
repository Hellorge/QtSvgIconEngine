#include <QApplication>
#include <QPushButton>
#include <QAction>
#include <QToolButton>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <utility>
#include "SvgIconEngine.h"
#include "SvgQIconEngine.h"

static int failures = 0;
static void check(const char *n, bool ok){ qInfo().noquote() << (ok?"  PASS  ":"  FAIL  ") << n; if(!ok) failures++; }
static QString dom(const QImage &img){
    QHash<QRgb,int> h;
    for(int y=0;y<img.height();++y) for(int x=0;x<img.width();++x){
        QColor c=img.pixelColor(x,y); if(c.alpha()>220) h[c.rgb()]++; }
    QRgb b=0; int n=0; for(auto it=h.begin();it!=h.end();++it) if(it.value()>n){n=it.value();b=it.key();}
    return n?QColor(b).name():QString("-");
}

int main(int argc,char**argv){
    QApplication app(argc,argv);
    const QString root = argv[1];

    QVariantMap o;
    o["size"] = QSize(64,64);
    o["color"]          = QColor("#94a3b8");
    o["hover_color"]    = QColor("#38bdf8");
    o["selected_color"] = QColor("#22c55e");
    o["hover_icon"]     = "heart";

    SvgIconEngine e(root);
    QVariantMap opts = o;
    QIcon icon = e.icon("regular/heart-half", opts);

    check("engine.icon() returns a non-null QIcon", !icon.isNull());
    check("QIcon reports a name", !icon.name().isEmpty());

    auto modeColor = [&](QIcon::Mode m){
        return dom(icon.pixmap(QSize(64,64), m, QIcon::Off).toImage());
    };
    check("QIcon::Normal   renders base colour",     modeColor(QIcon::Normal)   == "#94a3b8");
    check("QIcon::Active   renders hover colour",    modeColor(QIcon::Active)   == "#38bdf8");
    check("QIcon::Selected renders selected colour", modeColor(QIcon::Selected) == "#22c55e");

    // Disabled dims via opacity, so no fully-opaque dominant colour remains
    check("QIcon::Disabled is dimmed", modeColor(QIcon::Disabled) == "-");

    // Mode -> State mapping
    check("Active maps to Hovered",  SvgQIconEngine::toIconState(QIcon::Active)   == SvgIcon::Hovered);
    check("Selected maps to Selected", SvgQIconEngine::toIconState(QIcon::Selected) == SvgIcon::Selected);

    // Per-state artwork through QIcon: hover uses a different SVG, so the
    // rendered alpha coverage differs from Normal.
    auto coverage = [&](QIcon::Mode m){
        QImage im = icon.pixmap(QSize(64,64), m, QIcon::Off).toImage();
        int n=0; for(int y=0;y<im.height();++y) for(int x=0;x<im.width();++x) if(im.pixelColor(x,y).alpha()>128) ++n;
        return n;
    };
    check("QIcon::Active uses the per-state artwork (different coverage)",
          coverage(QIcon::Active) != coverage(QIcon::Normal));

    // Actually usable by stock Qt widgets
    QPushButton btn;
    btn.setIcon(icon);
    check("QPushButton::setIcon accepts it", !btn.icon().isNull());
    QAction act(icon, "Go", nullptr);
    check("QAction accepts it", !act.icon().isNull());
    QToolButton tb; tb.setIcon(icon);
    check("QToolButton accepts it", !tb.icon().isNull());

    // scalable: a 16px and a 128px request both render
    check("renders at 16px",  !icon.pixmap(QSize(16,16)).isNull());
    check("renders at 128px", !icon.pixmap(QSize(128,128)).isNull());

    // high-dpi
    QPixmap hd = icon.pixmap(QSize(32,32), 2.0, QIcon::Normal, QIcon::Off);
    check("high-dpi pixmap is backed by 2x device pixels", hd.size() == QSize(64,64));

    // Regression: QSvgRenderer::render(painter) targets painter->viewport() in
    // DEVICE pixels, so at dpr 2 the artwork was drawn 2x and clipped to its
    // top-left quarter. The rendered shape must be scale-invariant.
    // heart-half is asymmetric on purpose; use a symmetric glyph for this check.
    QVariantMap so; so["size"]=QSize(32,32); so["color"]=QColor("#ffffff");
    QIcon sym = e.icon("regular/heart", so);
    auto shape = [&](qreal dpr) {
        QImage im = sym.pixmap(QSize(32,32), dpr, QIcon::Normal, QIcon::Off).toImage();
        // fraction of opaque pixels in each horizontal half
        int left=0, right=0;
        for (int y=0;y<im.height();++y) for (int x=0;x<im.width();++x)
            if (im.pixelColor(x,y).alpha()>128) { (x < im.width()/2 ? left : right)++; }
        return std::pair<int,int>(left, right);
    };
    auto [l1,r1] = shape(1.0);
    auto [l2,r2] = shape(2.0);
    check("dpr 1: artwork is horizontally symmetric", qAbs(l1-r1) <= qMax(2, l1/10));
    check("dpr 2: artwork is horizontally symmetric (not clipped top-left)",
          qAbs(l2-r2) <= qMax(2, l2/10));
    check("dpr 2 coverage is ~4x dpr 1 (same shape, more pixels)",
          qAbs((l2+r2) - 4*(l1+r1)) < (l1+r1));

    qInfo().noquote() << (failures? QString("%1 FAILED").arg(failures) : QString("ALL QICON TESTS PASSED"));
    return failures;
}
