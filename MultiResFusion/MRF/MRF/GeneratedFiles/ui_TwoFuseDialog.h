/********************************************************************************
** Form generated from reading UI file 'TwoFuseDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.9.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TWOFUSEDIALOG_H
#define UI_TWOFUSEDIALOG_H

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

class Ui_TwoFuseDlg
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpinBox *spinBox;
    QWidget *layoutWidget1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *resLabel;
    QComboBox *resDirComboBox;
    QToolButton *resToolButton;
    QWidget *layoutWidget_2;
    QHBoxLayout *horizontalLayout_3;
    QLabel *fileLabel;
    QComboBox *fileDirComboBox;
    QToolButton *fileToolButton;

    void setupUi(QDialog *TwoFuseDlg)
    {
        if (TwoFuseDlg->objectName().isEmpty())
            TwoFuseDlg->setObjectName(QStringLiteral("TwoFuseDlg"));
        TwoFuseDlg->resize(568, 283);
        buttonBox = new QDialogButtonBox(TwoFuseDlg);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(190, 240, 341, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        layoutWidget = new QWidget(TwoFuseDlg);
        layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
        layoutWidget->setGeometry(QRect(70, 190, 301, 22));
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
        spinBox->setReadOnly(true);

        horizontalLayout->addWidget(spinBox);

        layoutWidget1 = new QWidget(TwoFuseDlg);
        layoutWidget1->setObjectName(QStringLiteral("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(30, 130, 481, 28));
        horizontalLayout_2 = new QHBoxLayout(layoutWidget1);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        resLabel = new QLabel(layoutWidget1);
        resLabel->setObjectName(QStringLiteral("resLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(resLabel->sizePolicy().hasHeightForWidth());
        resLabel->setSizePolicy(sizePolicy);
        resLabel->setAlignment(Qt::AlignCenter);
        resLabel->setWordWrap(true);

        horizontalLayout_2->addWidget(resLabel);

        resDirComboBox = new QComboBox(layoutWidget1);
        resDirComboBox->setObjectName(QStringLiteral("resDirComboBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(resDirComboBox->sizePolicy().hasHeightForWidth());
        resDirComboBox->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(resDirComboBox);

        resToolButton = new QToolButton(layoutWidget1);
        resToolButton->setObjectName(QStringLiteral("resToolButton"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(resToolButton->sizePolicy().hasHeightForWidth());
        resToolButton->setSizePolicy(sizePolicy2);

        horizontalLayout_2->addWidget(resToolButton);

        layoutWidget_2 = new QWidget(TwoFuseDlg);
        layoutWidget_2->setObjectName(QStringLiteral("layoutWidget_2"));
        layoutWidget_2->setGeometry(QRect(30, 50, 481, 28));
        horizontalLayout_3 = new QHBoxLayout(layoutWidget_2);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        fileLabel = new QLabel(layoutWidget_2);
        fileLabel->setObjectName(QStringLiteral("fileLabel"));
        sizePolicy.setHeightForWidth(fileLabel->sizePolicy().hasHeightForWidth());
        fileLabel->setSizePolicy(sizePolicy);
        fileLabel->setAlignment(Qt::AlignCenter);
        fileLabel->setWordWrap(true);

        horizontalLayout_3->addWidget(fileLabel);

        fileDirComboBox = new QComboBox(layoutWidget_2);
        fileDirComboBox->setObjectName(QStringLiteral("fileDirComboBox"));
        sizePolicy1.setHeightForWidth(fileDirComboBox->sizePolicy().hasHeightForWidth());
        fileDirComboBox->setSizePolicy(sizePolicy1);

        horizontalLayout_3->addWidget(fileDirComboBox);

        fileToolButton = new QToolButton(layoutWidget_2);
        fileToolButton->setObjectName(QStringLiteral("fileToolButton"));
        sizePolicy2.setHeightForWidth(fileToolButton->sizePolicy().hasHeightForWidth());
        fileToolButton->setSizePolicy(sizePolicy2);

        horizontalLayout_3->addWidget(fileToolButton);


        retranslateUi(TwoFuseDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), TwoFuseDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), TwoFuseDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(TwoFuseDlg);
    } // setupUi

    void retranslateUi(QDialog *TwoFuseDlg)
    {
        TwoFuseDlg->setWindowTitle(QApplication::translate("TwoFuseDlg", "\344\270\211\347\273\264\351\207\215\345\273\272", Q_NULLPTR));
        label_2->setText(QApplication::translate("TwoFuseDlg", "\344\277\235\345\255\230\345\233\276\347\211\207\345\260\272\345\257\270:", Q_NULLPTR));
        resLabel->setText(QApplication::translate("TwoFuseDlg", "\345\255\230\346\224\276\347\273\223\346\236\234\350\267\257\345\276\204:", Q_NULLPTR));
        resToolButton->setText(QApplication::translate("TwoFuseDlg", "...", Q_NULLPTR));
        fileLabel->setText(QApplication::translate("TwoFuseDlg", "\344\275\216\345\210\206\350\276\250\347\216\207\345\244\247\345\255\224\345\272\217\345\210\227\345\233\276\350\267\257\345\276\204:", Q_NULLPTR));
        fileToolButton->setText(QApplication::translate("TwoFuseDlg", "...", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class TwoFuseDlg: public Ui_TwoFuseDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TWOFUSEDIALOG_H
