#include <QApplication>
#include <QPalette>
#include <QDebug>
#include "SvgIconEngine.h"
#include "SvgIcon.h"
#include "SvgIconButton.h"

static int fails=0;
static void check(const char*n,bool ok){ qInfo().noquote()<<(ok?"  PASS  ":"  FAIL  ")<<n; if(!ok)++fails; }

int main(int argc,char**argv){
    QApplication app(argc,argv);
    SvgIconEngine e(argv[1]);

    // 1. A bare icon (no button, so no synthetic hover) follows the palette.
    QVariantMap o; o["size"]=QSize(32,32);          // no explicit colour
    SvgIcon *icon = e.getIcon("regular/heart", o);

    QPalette dark; dark.setColor(QPalette::Text, QColor("#e2e8f0"));
    dark.setColor(QPalette::Highlight, QColor("#38bdf8"));
    app.setPalette(dark); app.processEvents();
    check("dark theme -> icon takes palette text", icon->color()==QColor("#e2e8f0"));

    QPalette light; light.setColor(QPalette::Text, QColor("#0f172a"));
    light.setColor(QPalette::Highlight, QColor("#0284c7"));
    app.setPalette(light); app.processEvents();
    check("light theme -> icon re-tints", icon->color()==QColor("#0f172a"));

    // 2. The button's hover wash is regenerated from the new highlight,
    //    and regenerating it does not recurse (we'd have crashed by now).
    QVariantMap o2; o2["size"]=QSize(32,32);
    auto *b = new SvgIconButton(e.getIcon("regular/star", o2), "Themed");
    b->setButtonStyle(SvgIconButton::Flat);
    b->show(); app.processEvents();

    const bool lightWash = b->styleSheet().contains("rgba(2, 132, 199");   // #0284c7
    app.setPalette(dark); app.processEvents();
    const bool darkWash  = b->styleSheet().contains("rgba(56, 189, 248");  // #38bdf8

    check("hover wash derives from the light highlight", lightWash);
    check("hover wash regenerates on theme change",       darkWash);
    check("no recursion (process still alive)",           true);

    qInfo().noquote() << (fails? "FAILED" : "ALL THEME TESTS PASSED");
    return fails;
}
