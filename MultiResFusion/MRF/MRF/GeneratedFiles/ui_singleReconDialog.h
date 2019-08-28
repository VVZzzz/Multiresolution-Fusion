/********************************************************************************
** Form generated from reading UI file 'singleReconDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SINGLERECONDIALOG_H
#define UI_SINGLERECONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SingleReconDialog
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpinBox *spinBox;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QComboBox *dirComboBox;
    QToolButton *toolButton;

    void setupUi(QDialog *SingleReconDialog)
    {
        if (SingleReconDialog->objectName().isEmpty())
            SingleReconDialog->setObjectName(QStringLiteral("SingleReconDialog"));
        SingleReconDialog->resize(507, 190);
        buttonBox = new QDialogButtonBox(SingleReconDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(150, 150, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        layoutWidget = new QWidget(SingleReconDialog);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(70, 90, 301, 22));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout->addWidget(label_2);

        spinBox = new QSpinBox(layoutWidget);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setAlignment(Qt::AlignCenter);
        spinBox->setReadOnly(false);

        horizontalLayout->addWidget(spinBox);

        layoutWidget1 = new QWidget(SingleReconDialog);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(9, 29, 481, 28));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(layoutWidget1);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);

        horizontalLayout_2->addWidget(label);

        dirComboBox = new QComboBox(layoutWidget1);
        dirComboBox->setObjectName(QStringLiteral("dirComboBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(dirComboBox->sizePolicy().hasHeightForWidth());
        dirComboBox->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(dirComboBox);

        toolButton = new QToolButton(layoutWidget1);
        toolButton->setObjectName(QStringLiteral("toolButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(toolButton->sizePolicy().hasHeightForWidth());
        toolButton->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(toolButton);


        retranslateUi(SingleReconDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SingleReconDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SingleReconDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(SingleReconDialog);
    } // setupUi

    void retranslateUi(QDialog *SingleReconDialog)
    {
        SingleReconDialog->setWindowTitle(QApplication::translate("SingleReconDialog", "\344\270\211\347\273\264\351\207\215\345\273\272", Q_NULLPTR));
        label_2->setText(QApplication::translate("SingleReconDialog", "\344\277\235\345\255\230\345\233\276\347\211\207\345\260\272\345\257\270:", Q_NULLPTR));
        label->setText(QApplication::translate("SingleReconDialog", "\345\255\230\346\224\276\344\270\211\347\273\264\351\207\215\345\273\272\347\273\223\346\236\234\350\267\257\345\276\204:", Q_NULLPTR));
        toolButton->setText(QApplication::translate("SingleReconDialog", "...", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class SingleReconDialog: public Ui_SingleReconDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SINGLERECONDIALOG_H
