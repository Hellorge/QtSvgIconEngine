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

#include "SvgIconEngine.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QLoggingCategory::setFilterRules(
    	"svgiconengine.debug=true\n"
        "svgiconengine.warning=true\n"
        "svgiconengine.critical=true"
    );

    QVariantMap colors;
    colors["color"] = QColor(Qt::blue);
    colors["size"] = QSize(64, 64);
    // colors["background"] = QColor(Qt::green);
    SvgIconEngine iconEngine(":/icons", colors);

    QList<SvgIcon*> icons;
    icons.append(iconEngine.getIcon("regular", "calendar", colors));

    colors["shadow"] = true;
    icons.append(iconEngine.getIcon("regular", "flag"));

    colors["color"] = QColor(Qt::red);
    colors["opacity"] = .6;
    colors["border_width"] = 100;

    icons.append(iconEngine.getIcon("regular", "woman", colors));

    icons.append(iconEngine.getIcon("regular", "calendar", colors));

    QWidget mainWindow;
    QVBoxLayout *mainLayout = new QVBoxLayout(&mainWindow);

    for (int i = 0; i < icons.count() ; i++) {
		mainLayout->addWidget(icons[i]);
    }

    mainWindow.show();

    // QWidget *scrollWidget = new QWidget;
    // QGridLayout *gridLayout = new QGridLayout(scrollWidget);

    // qreal iconSize = 64;

    // QVector<QIcon> icons;
    // QVector<QString> iconNames;

    // QStringList styles = {"regular"};
    // for (const QString &style : styles) {
    //     QDir dir(":/icons/" + style);
    //     QStringList files = dir.entryList(QStringList() << "*.svg", QDir::Files);

    //     for (const QString &file : files) {
    //         QString iconName = file.section('.', 0, 0);
    //         QVariantMap colors;
    //         colors["default"] = QColor(QApplication::palette().text().color());
    //         SvgIcon icon = iconEngine.getIcon(style, iconName, colors);

    //         icons.append(icon);
    //         iconNames.append(iconName);

    //         icon.animateColorChange(Qt::red, Qt::blue, 1000);
    //         icon.rotate(45, 500);
    //     }
    // }


    // const int numColumns = 5;
    // int numRows = (icons.size() + numColumns - 1) / numColumns;

    // for (int i = 0; i < icons.size(); ++i) {
    //     int row = i / numColumns;
    //     int col = i % numColumns;

    //     QWidget *iconWidget = new QWidget;
    //     QVBoxLayout *vLayout = new QVBoxLayout(iconWidget);

    //     QLabel *iconLabel = new QLabel;
    //     iconLabel->setPixmap(icons[i].pixmap(iconSize, iconSize));
    //     iconLabel->setAlignment(Qt::AlignCenter);

    //     QLabel *nameLabel = new QLabel(iconNames[i]);
    //     nameLabel->setAlignment(Qt::AlignCenter);

    //     vLayout->addWidget(iconLabel);
    //     vLayout->addWidget(nameLabel);
    //     vLayout->setAlignment(Qt::AlignCenter);

    //     gridLayout->addWidget(iconWidget, row, col);
    // }

    // scrollWidget->setLayout(gridLayout);

    // QScrollArea *scrollArea = new QScrollArea;
    // scrollArea->setWidget(scrollWidget);
    // scrollArea->setWidgetResizable(true);

    // mainLayout->addWidget(scrollArea);

    // mainWindow.setLayout(mainLayout);
    // mainWindow.show();

    return app.exec();
}
