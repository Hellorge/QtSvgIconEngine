#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QIcon>
#include <QPixmap>
#include <QVariantMap>
#include <QDir>
#include <QDebug>
#include <QVector>
#include <QPushButton>
#include <QParallelAnimationGroup>

#include "SvgIconEngine.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QLoggingCategory::setFilterRules(
    	"svgiconengine.debug=true\n"
        "svgiconengine.warning=true\n"
        "svgiconengine.critical=true"
    );

    QVariantMap colors;
    colors["default_colors"] = true;
    colors["color"] = QColor(Qt::blue);
    colors["size"] = QSize(64, 64);
    colors["scale"] = .5;
    SvgIconEngine iconEngine(":/icons", colors);

    QList<QIcon> icons;
    icons.append(iconEngine.getIcon("regular", "calendar", colors));

    // colors["shadow"] = true;
    icons.append(iconEngine.getIcon("regular", "flag"));

    colors["color"] = QColor(Qt::red);
    colors["opacity"] = .6;
    // colors["border_width"] = 100;

    icons.append(iconEngine.getIcon("regular", "woman", colors));

    colors["background"] = QColor(Qt::green);
    icons.append(iconEngine.getIcon("regular", "calendar", colors));

    QWidget mainWindow;
    QVBoxLayout *mainLayout = new QVBoxLayout(&mainWindow);

    for (int i = 0; i < icons.count() ; i++) {
        QPushButton *btn = new QPushButton(icons[i], "");
        btn->setIconSize(QSize(64, 64));
        mainLayout->addWidget(btn);
        // mainLayout->addWidget(icons[i]);
    }

    mainWindow.show();

    // QPropertyAnimation *colorAnimation = new QPropertyAnimation(icons[0], "color");
    // colorAnimation->setDuration(1000); // Animation duration in milliseconds
    // colorAnimation->setStartValue(QColor(Qt::red));
    // colorAnimation->setEndValue(QColor(Qt::blue));
    // colorAnimation->setEasingCurve(QEasingCurve::Linear);
    // colorAnimation->setLoopCount(-1); // Loop indefinitely

    // QPropertyAnimation *opacityAnimation = new QPropertyAnimation(icons[1], "opacity");
    // opacityAnimation->setDuration(1000);
    // opacityAnimation->setStartValue(1.0);
    // opacityAnimation->setEndValue(0);
    // opacityAnimation->setEasingCurve(QEasingCurve::Linear);
    // opacityAnimation->setLoopCount(-1);

    // QPropertyAnimation *scaleAnimation = new QPropertyAnimation(icons[3], "scale");
    // scaleAnimation->setDuration(1000);
    // scaleAnimation->setStartValue(1.0);
    // scaleAnimation->setEndValue(.5);
    // scaleAnimation->setEasingCurve(QEasingCurve::Linear);
    // scaleAnimation->setLoopCount(-1);

    // QParallelAnimationGroup *group = new QParallelAnimationGroup;
    // group->addAnimation(colorAnimation);
    // group->addAnimation(opacityAnimation);
    // group->addAnimation(scaleAnimation);
    // group->start();

    return app.exec();
}
