/********************************************************************************
** Form generated from reading UI file 'ribbontabcontent.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RIBBONTABCONTENT_H
#define UI_RIBBONTABCONTENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RibbonTabContent
{
public:
    QGridLayout *gridLayout;
    QScrollArea *ribbonTabScrollArea;
    QWidget *ribbonTabScrollAreaContent;
    QGridLayout *gridLayout_2;
    QWidget *spacer;
    QHBoxLayout *ribbonHorizontalLayout;

    void setupUi(QWidget *RibbonTabContent)
    {
        if (RibbonTabContent->objectName().isEmpty())
            RibbonTabContent->setObjectName(QStringLiteral("RibbonTabContent"));
        RibbonTabContent->resize(400, 90);
        gridLayout = new QGridLayout(RibbonTabContent);
        gridLayout->setSpacing(0);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ribbonTabScrollArea = new QScrollArea(RibbonTabContent);
        ribbonTabScrollArea->setObjectName(QStringLiteral("ribbonTabScrollArea"));
        ribbonTabScrollArea->setFrameShape(QFrame::NoFrame);
        ribbonTabScrollArea->setFrameShadow(QFrame::Plain);
        ribbonTabScrollArea->setLineWidth(0);
        ribbonTabScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ribbonTabScrollArea->setWidgetResizable(true);
        ribbonTabScrollAreaContent = new QWidget();
        ribbonTabScrollAreaContent->setObjectName(QStringLiteral("ribbonTabScrollAreaContent"));
        ribbonTabScrollAreaContent->setGeometry(QRect(0, 0, 400, 90));
        gridLayout_2 = new QGridLayout(ribbonTabScrollAreaContent);
        gridLayout_2->setSpacing(0);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        spacer = new QWidget(ribbonTabScrollAreaContent);
        spacer->setObjectName(QStringLiteral("spacer"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(spacer->sizePolicy().hasHeightForWidth());
        spacer->setSizePolicy(sizePolicy);

        gridLayout_2->addWidget(spacer, 0, 1, 1, 1);

        ribbonHorizontalLayout = new QHBoxLayout();
        ribbonHorizontalLayout->setSpacing(0);
        ribbonHorizontalLayout->setObjectName(QStringLiteral("ribbonHorizontalLayout"));
        ribbonHorizontalLayout->setContentsMargins(0, 3, 0, 0);

        gridLayout_2->addLayout(ribbonHorizontalLayout, 0, 0, 1, 1);

        ribbonTabScrollArea->setWidget(ribbonTabScrollAreaContent);

        gridLayout->addWidget(ribbonTabScrollArea, 0, 0, 1, 1);


        retranslateUi(RibbonTabContent);

        QMetaObject::connectSlotsByName(RibbonTabContent);
    } // setupUi

    void retranslateUi(QWidget *RibbonTabContent)
    {
        RibbonTabContent->setWindowTitle(QApplication::translate("RibbonTabContent", "Form", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class RibbonTabContent: public Ui_RibbonTabContent {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RIBBONTABCONTENT_H
