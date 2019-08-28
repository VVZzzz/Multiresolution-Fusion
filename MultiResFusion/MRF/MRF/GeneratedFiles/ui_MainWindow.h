/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <MyGraphicsView.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <ribbon.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QFrame *line;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QListWidget *listWidget;
    MyGraphicsView *graphicsView;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QProgressBar *progressBar;
    QDockWidget *ribbonDockWidget;
    QWidget *ribbonDockWidgetContents;
    QGridLayout *gridLayout;
    Ribbon *ribbonTabWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(799, 633);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindowClass->sizePolicy().hasHeightForWidth());
        MainWindowClass->setSizePolicy(sizePolicy);
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy1);
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        line = new QFrame(centralWidget);
        line->setObjectName(QStringLiteral("line"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
        line->setSizePolicy(sizePolicy2);
        line->setMinimumSize(QSize(0, 3));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(9);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, -1, -1, -1);
        listWidget = new QListWidget(centralWidget);
        listWidget->setObjectName(QStringLiteral("listWidget"));
        sizePolicy1.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy1);
        listWidget->setFrameShape(QFrame::StyledPanel);
        listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        listWidget->setAutoScroll(true);
        listWidget->setAutoScrollMargin(16);
        listWidget->setDragDropMode(QAbstractItemView::DragDrop);
        listWidget->setMovement(QListView::Snap);
        listWidget->setResizeMode(QListView::Adjust);

        horizontalLayout->addWidget(listWidget);

        graphicsView = new MyGraphicsView(centralWidget);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setContextMenuPolicy(Qt::CustomContextMenu);
        graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        horizontalLayout->addWidget(graphicsView);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addLayout(verticalLayout);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setSizeConstraint(QLayout::SetDefaultConstraint);
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label);

        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        sizePolicy2.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
        progressBar->setSizePolicy(sizePolicy2);
        progressBar->setValue(24);

        horizontalLayout_4->addWidget(progressBar);


        verticalLayout_2->addLayout(horizontalLayout_4);

        MainWindowClass->setCentralWidget(centralWidget);
        ribbonDockWidget = new QDockWidget(MainWindowClass);
        ribbonDockWidget->setObjectName(QStringLiteral("ribbonDockWidget"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(ribbonDockWidget->sizePolicy().hasHeightForWidth());
        ribbonDockWidget->setSizePolicy(sizePolicy3);
        ribbonDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        ribbonDockWidget->setAllowedAreas(Qt::TopDockWidgetArea);
        ribbonDockWidgetContents = new QWidget();
        ribbonDockWidgetContents->setObjectName(QStringLiteral("ribbonDockWidgetContents"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(ribbonDockWidgetContents->sizePolicy().hasHeightForWidth());
        ribbonDockWidgetContents->setSizePolicy(sizePolicy4);
        gridLayout = new QGridLayout(ribbonDockWidgetContents);
        gridLayout->setSpacing(0);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ribbonTabWidget = new Ribbon(ribbonDockWidgetContents);
        ribbonTabWidget->setObjectName(QStringLiteral("ribbonTabWidget"));
        QSizePolicy sizePolicy5(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(ribbonTabWidget->sizePolicy().hasHeightForWidth());
        ribbonTabWidget->setSizePolicy(sizePolicy5);

        gridLayout->addWidget(ribbonTabWidget, 0, 0, 1, 1);

        ribbonDockWidget->setWidget(ribbonDockWidgetContents);
        MainWindowClass->addDockWidget(static_cast<Qt::DockWidgetArea>(4), ribbonDockWidget);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        sizePolicy.setHeightForWidth(statusBar->sizePolicy().hasHeightForWidth());
        statusBar->setSizePolicy(sizePolicy);
        MainWindowClass->setStatusBar(statusBar);

        retranslateUi(MainWindowClass);

        ribbonTabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "MainWindow", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindowClass", "TextLabel", Q_NULLPTR));
        ribbonDockWidget->setWindowTitle(QApplication::translate("MainWindowClass", "Ribbon", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
