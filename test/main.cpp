#include <QApplication>
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
#include <QMenu>
#include <QMessageBox>
#include <QObject>

#include "SvgIconEngine.h"
#include "SvgIconButton.h"

// Helper function to create a section with title
QWidget* createSection(const QString& title, QWidget* content) {
    QWidget* section = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(section);
    QLabel* label = new QLabel(title);
    label->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(label);
    layout->addWidget(content);
    return section;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QLoggingCategory::setFilterRules(
        "svgiconengine.debug=true\n"
        "svgiconengine.warning=true\n"
        "svgiconengine.critical=true"
    );

    // Initialize icon engine with default options
    QVariantMap colors;
    colors["default_colors"] = true;
    colors["size"] = QSize(32, 32);  // Smaller default size for buttons
    colors["scale"] = 1.0;
    SvgIconEngine iconEngine(":/icons", colors);

    // Main window setup
    QWidget mainWindow;
    mainWindow.setWindowTitle("SVG Icon Button Demo");
    QVBoxLayout* mainLayout = new QVBoxLayout(&mainWindow);

    // Create grid for different button types
    QWidget* buttonGrid = new QWidget;
    QGridLayout* gridLayout = new QGridLayout(buttonGrid);

    // 1. Basic Buttons Section
    {
        QWidget* container = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(container);

        // Regular button
        SvgIcon* calendarIcon = iconEngine.getIcon("regular", "calendar", colors);
        SvgIconButton* basicBtn = new SvgIconButton(calendarIcon, "Calendar");
        basicBtn->setButtonStyle(SvgIconButton::Default);
        QObject::connect(basicBtn, &SvgIconButton::clicked, []() {
            QMessageBox::information(nullptr, "Click", "Basic button clicked!");
        });
        layout->addWidget(basicBtn);

        // Flat button
        SvgIcon* flagIcon = iconEngine.getIcon("regular", "flag", colors);
        SvgIconButton* flatBtn = new SvgIconButton(flagIcon, "Flag");
        flatBtn->setButtonStyle(SvgIconButton::Flat);
        layout->addWidget(flatBtn);

        // Outline button
        SvgIcon* userIcon = iconEngine.getIcon("regular", "woman", colors);
        SvgIconButton* outlineBtn = new SvgIconButton(userIcon, "User");
        outlineBtn->setButtonStyle(SvgIconButton::Outline);
        layout->addWidget(outlineBtn);

        mainLayout->addWidget(createSection("Basic Buttons", container));
    }

    // 2. Dropdown Buttons Section
    {
        QWidget* container = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(container);

        // Dropdown button
        SvgIcon* menuIcon = iconEngine.getIcon("regular", "calendar", colors);
        SvgIconButton* dropdownBtn = new SvgIconButton(menuIcon, "Options");
        QMenu* menu = new QMenu;
        menu->addAction("Option 1");
        menu->addAction("Option 2");
        menu->addAction("Option 3");
        dropdownBtn->makeDropdown(menu);
        layout->addWidget(dropdownBtn);

        // Menu button with actions
        SvgIcon* actionIcon = iconEngine.getIcon("regular", "flag", colors);
        SvgIconButton* actionBtn = new SvgIconButton(actionIcon, "Actions");
        QList<QAction*> actions;
        actions << new QAction("Action 1") << new QAction("Action 2");
        actionBtn->makeMenu(actions);
        layout->addWidget(actionBtn);

        mainLayout->addWidget(createSection("Dropdown Buttons", container));
    }

    // 3. Toggle Buttons Section
    {
        QWidget* container = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(container);

        // Toggle button
        SvgIcon* toggleIcon = iconEngine.getIcon("regular", "woman", colors);
        SvgIconButton* toggleBtn = new SvgIconButton(toggleIcon, "Toggle");
        toggleBtn->makeToggleable(true);
        QObject::connect(toggleBtn, &SvgIconButton::toggled, [](bool checked) {
            qDebug() << "Toggle state:" << checked;
        });
        layout->addWidget(toggleBtn);

        mainLayout->addWidget(createSection("Toggle Buttons", container));
    }

    // 4. Tool Buttons Section
    {
        QWidget* container = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(container);

        // Tool buttons
        for (const QString& iconName : {"calendar", "flag", "woman"}) {
            SvgIcon* icon = iconEngine.getIcon("regular", iconName, colors);
            SvgIconButton* toolBtn = new SvgIconButton(icon);
            toolBtn->makeToolButton();
            layout->addWidget(toolBtn);
        }

        mainLayout->addWidget(createSection("Tool Buttons", container));
    }

    // 5. Animated Button Section
    {
        QWidget* container = new QWidget;
        QHBoxLayout* layout = new QHBoxLayout(container);

        SvgIcon* animIcon = iconEngine.getIcon("regular", "calendar", colors);
        SvgIconButton* animBtn = new SvgIconButton(animIcon, "Animate");
        layout->addWidget(animBtn);

        // Create color animation
        QPropertyAnimation* colorAnimation = new QPropertyAnimation(animIcon, "color");
        colorAnimation->setDuration(1000);
        colorAnimation->setStartValue(QColor(Qt::red));
        colorAnimation->setEndValue(QColor(Qt::blue));
        colorAnimation->setLoopCount(-1);

        // Create opacity animation
        QPropertyAnimation* opacityAnimation = new QPropertyAnimation(animIcon, "opacity");
        opacityAnimation->setDuration(1000);
        opacityAnimation->setStartValue(1.0);
        opacityAnimation->setEndValue(0.5);
        opacityAnimation->setLoopCount(-1);

        QParallelAnimationGroup* group = new QParallelAnimationGroup;
        group->addAnimation(colorAnimation);
        group->addAnimation(opacityAnimation);
        group->start();

        mainLayout->addWidget(createSection("Animated Button", container));
    }

    // Add some spacing and margins
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Set a minimum size for the window
    mainWindow.setMinimumSize(600, 400);
    mainWindow.show();

    return app.exec();
}
