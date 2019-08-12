/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <ribbon.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QWidget *centralWidget;
    QDockWidget *ribbonDockWidget;
    QWidget *ribbonDockWidgetContents;
    QGridLayout *gridLayout;
    Ribbon *ribbonTabWidget;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(800, 600);
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindowClass->setCentralWidget(centralWidget);
        ribbonDockWidget = new QDockWidget(MainWindowClass);
        ribbonDockWidget->setObjectName(QStringLiteral("ribbonDockWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ribbonDockWidget->sizePolicy().hasHeightForWidth());
        ribbonDockWidget->setSizePolicy(sizePolicy);
        ribbonDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        ribbonDockWidget->setAllowedAreas(Qt::TopDockWidgetArea);
        ribbonDockWidgetContents = new QWidget();
        ribbonDockWidgetContents->setObjectName(QStringLiteral("ribbonDockWidgetContents"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ribbonDockWidgetContents->sizePolicy().hasHeightForWidth());
        ribbonDockWidgetContents->setSizePolicy(sizePolicy1);
        gridLayout = new QGridLayout(ribbonDockWidgetContents);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ribbonTabWidget = new Ribbon(ribbonDockWidgetContents);
        ribbonTabWidget->setObjectName(QStringLiteral("ribbonTabWidget"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(ribbonTabWidget->sizePolicy().hasHeightForWidth());
        ribbonTabWidget->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(ribbonTabWidget, 0, 0, 1, 1);

        ribbonDockWidget->setWidget(ribbonDockWidgetContents);
        MainWindowClass->addDockWidget(static_cast<Qt::DockWidgetArea>(4), ribbonDockWidget);

        retranslateUi(MainWindowClass);

        ribbonTabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "MainWindow", Q_NULLPTR));
        ribbonDockWidget->setWindowTitle(QApplication::translate("MainWindowClass", "Ribbon", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
