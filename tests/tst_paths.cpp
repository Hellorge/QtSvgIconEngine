#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "SvgIconEngine.h"
#include "SvgIcon.h"

static int fails=0;
static void check(const char*n,bool ok){ qInfo().noquote()<<(ok?"  PASS  ":"  FAIL  ")<<n; if(!ok)++fails; }

static const char *kSprite =
  "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 10'>"
  "<rect id='a' x='0' y='0' width='10' height='10' fill='#000'/>"
  "<circle id='b' cx='15' cy='5' r='5' fill='#000'/></svg>";

int main(int argc,char**argv){
    QApplication app(argc,argv);
    const QString root = argv[1];
    SvgIconEngine e(root);

    // --- resolution rules --------------------------------------------------
    check("root-relative path gains .svg",
          e.resolvePath("regular/heart") == root + "/regular/heart.svg");
    check("explicit .svg is not doubled",
          e.resolvePath("regular/heart.svg") == root + "/regular/heart.svg");
    check("absolute path is verbatim",
          e.resolvePath("/tmp/x.svg") == "/tmp/x.svg");
    check("qrc path is verbatim",
          e.resolvePath(":/icons/x.svg") == ":/icons/x.svg");
    check("bare name with baseDir resolves as a sibling",
          e.resolvePath("heart", root + "/regular") == root + "/regular/heart.svg");
    check("path with a slash ignores baseDir (root-relative)",
          e.resolvePath("solid/heart", root + "/regular") == root + "/solid/heart.svg");
    check("empty stays empty", e.resolvePath("").isEmpty());

    // --- the paths actually load ------------------------------------------
    QVariantMap o; o["size"]=QSize(32,32);
    check("root-relative loads", e.getIcon("regular/heart", o) != nullptr);

    QVariantMap o2; o2["size"]=QSize(32,32);
    check("absolute path loads", e.getIcon(root + "/solid/heart.svg", o2) != nullptr);

    // hover_icon sibling vs cross-style
    QVariantMap o3; o3["size"]=QSize(32,32); o3["hover_icon"]="heart";        // sibling
    SvgIcon *sib = e.getIcon("regular/heart-half", o3);
    check("hover_icon bare name = sibling", sib && sib->hasStateRenderer(SvgIcon::Hovered));

    QVariantMap o4; o4["size"]=QSize(32,32); o4["hover_icon"]="solid/heart";  // cross-style
    SvgIcon *cross = e.getIcon("regular/heart-half", o4);
    check("hover_icon with slash = root-relative", cross && cross->hasStateRenderer(SvgIcon::Hovered));

    // --- sprites, widget and QIcon ----------------------------------------
    QTemporaryDir tmp;
    const QString spritePath = tmp.path() + "/sheet.svg";
    {
        QFile f(spritePath);
        check("sprite fixture is writable", f.open(QIODevice::WriteOnly));
        f.write(kSprite);
    }

    QVariantMap so; so["size"]=QSize(32,32); so["color"]=QColor("#ff0000");
    SvgIcon *sprA = e.getIconFromSprite(spritePath, "a", so);
    check("sprite element -> widget", sprA != nullptr);

    QVariantMap so2; so2["size"]=QSize(32,32);
    check("missing sprite element returns nullptr",
          e.getIconFromSprite(spritePath, "zzz", so2) == nullptr);

    QVariantMap so3; so3["color"]=QColor("#ff0000");
    QIcon qa = e.iconFromSprite(spritePath, "a", so3);
    QVariantMap so4; so4["color"]=QColor("#ff0000");
    QIcon qb = e.iconFromSprite(spritePath, "b", so4);
    check("sprite element -> QIcon (a)", !qa.isNull());
    check("sprite element -> QIcon (b)", !qb.isNull());

    // 'a' is a square, 'b' is a circle: different coverage proves the element
    // is honoured rather than the whole sheet being drawn.
    auto coverage = [](const QIcon &i){
        QImage im = i.pixmap(QSize(32,32)).toImage();
        int n=0; for(int y=0;y<im.height();++y) for(int x=0;x<im.width();++x)
            if(im.pixelColor(x,y).alpha()>128) ++n;
        return n;
    };
    const int ca = coverage(qa), cb = coverage(qb);
    qInfo().noquote() << QString("         square=%1px circle=%2px").arg(ca).arg(cb);
    check("QIcon sprite renders the requested element, not the sheet", ca > 0 && cb > 0 && ca != cb);

    QIcon whole = e.icon(spritePath);
    check("whole sheet differs from a single element", coverage(whole) != ca);

    qInfo().noquote() << (fails? QString("%1 FAILED").arg(fails) : QString("ALL PATH/SPRITE TESTS PASSED"));
    return fails;
}
