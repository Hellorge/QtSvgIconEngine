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

    SvgIconEngine iconEngine(":/icons");

    QVariantMap colors;
    SvgIcon icon = iconEngine.getIcon("regular", "calendar", colors);
    colors["color"] = QColor(Qt::red);
    SvgIcon icon2 = iconEngine.getIcon("regular", "flag", colors);
    colors["default_colors"] = true;
    SvgIcon icon3 = iconEngine.getIcon("regular", "woman", colors);

    // icon.animateColorChange(Qt::red, Qt::blue, 1000);
    // icon.rotate(45, 500);

    QWidget mainWindow;
    QVBoxLayout *mainLayout = new QVBoxLayout(&mainWindow);
    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(icon.pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel2 = new QLabel;
    iconLabel2->setPixmap(icon2.pixmap(64, 64));
    iconLabel2->setAlignment(Qt::AlignCenter);

    QLabel *iconLabel3 = new QLabel;
    iconLabel3->setPixmap(icon3.pixmap(64, 64));
    iconLabel3->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(iconLabel2);
    mainLayout->addWidget(iconLabel3);
    mainWindow.setLayout(mainLayout);
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
