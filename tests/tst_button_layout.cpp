#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QDebug>
#include "SvgIconEngine.h"
#include "SvgIconButton.h"

static int failures = 0;
static void check(const char*n, bool ok){ qInfo().noquote()<<(ok?"  PASS  ":"  FAIL  ")<<n; if(!ok) failures++; }

int main(int argc,char**argv){
    QApplication app(argc,argv);
    SvgIconEngine e(argv[1]);
    QVariantMap o; o["size"]=QSize(32,32); o["color"]=QColor("#e2e8f0");

    QWidget root; root.setStyleSheet("background:#0f172a; color:#e2e8f0;");
    auto *v=new QVBoxLayout(&root);
    auto *r1=new QWidget; auto *h1=new QHBoxLayout(r1);
    auto *r2=new QWidget; auto *h2=new QHBoxLayout(r2);

    struct C { const char *text; SvgIconButton::ButtonStyle st; bool menu; bool icon; };
    const C cases[] = {
        {"Calendar",              SvgIconButton::Default, false, true},
        {"Hover swaps SVG",       SvgIconButton::Outline, false, true},
        {"Options",               SvgIconButton::Default, true,  true},
        {"Disabled QIcon",        SvgIconButton::Flat,    false, true},
        {"A Very Long Label Indeed", SvgIconButton::Outline, true, true},
        {"",                      SvgIconButton::Flat,    false, true},   // icon only
        {"No Icon Here",          SvgIconButton::Default, false, false},  // text only
        {"Menu+Long Text",        SvgIconButton::Flat,    true,  true},
    };

    QVector<SvgIconButton*> btns;
    int i=0;
    for (auto &c : cases) {
        QVariantMap oo=o;
        SvgIcon *ic = c.icon ? e.getIcon("regular/calendar", oo) : nullptr;
        auto *b = new SvgIconButton(ic, c.text);
        b->setButtonStyle(c.st);
        if (c.menu) { auto *m=new QMenu(b); m->addAction("One"); b->makeDropdown(m); }
        (i<4?h1:h2)->addWidget(b);
        btns.push_back(b); ++i;
    }
    v->addWidget(r1); v->addWidget(r2);
    root.resize(900,160); root.show(); app.processEvents();

    for (int j=0;j<btns.size();++j) {
        SvgIconButton *b = btns[j];
        const QString t = cases[j].text;
        const QFontMetrics fm(b->font());
        const int iconW = cases[j].icon ? 32 : 0;
        const int gap   = (iconW && !t.isEmpty()) ? 8 : 0;
        const int arrow = cases[j].menu ? 16 : 0;
        const int needed = iconW + gap + (t.isEmpty()?0:fm.horizontalAdvance(t)) + arrow;
        const bool fits = b->width() >= needed;
        const QString label = t.isEmpty() ? QStringLiteral("(icon only)") : t;
        qInfo().noquote() << QString("  %1 w=%2 needs>=%3").arg(label,-24).arg(b->width()).arg(needed);
        if (!fits) failures++;
    }
    check("every button is at least as wide as its content", failures==0);

    
    qInfo().noquote() << (failures? "LAYOUT FAILURES" : "LAYOUT OK");
    return failures;
}
