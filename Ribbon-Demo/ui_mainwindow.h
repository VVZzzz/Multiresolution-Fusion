/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
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
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include "ribbon.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QTextEdit *textEdit;
    QStatusBar *statusBar;
    QDockWidget *ribbonDockWidget;
    QWidget *ribbonDockWidgetContents;
    QGridLayout *gridLayout;
    Ribbon *ribbonTabWidget;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(800, 600);
        MainWindow->setStyleSheet(QStringLiteral("QTabWidget::pane { border: 0; }"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));

        gridLayout_2->addWidget(textEdit, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);
        ribbonDockWidget = new QDockWidget(MainWindow);
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
        sizePolicy.setHeightForWidth(ribbonDockWidgetContents->sizePolicy().hasHeightForWidth());
        ribbonDockWidgetContents->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(ribbonDockWidgetContents);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ribbonTabWidget = new Ribbon(ribbonDockWidgetContents);
        ribbonTabWidget->setObjectName(QStringLiteral("ribbonTabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(ribbonTabWidget->sizePolicy().hasHeightForWidth());
        ribbonTabWidget->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(ribbonTabWidget, 0, 0, 1, 1);

        ribbonDockWidget->setWidget(ribbonDockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(4), ribbonDockWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Qt-Ribbon-Widget", Q_NULLPTR));
        ribbonDockWidget->setWindowTitle(QApplication::translate("MainWindow", "Ribbon", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
