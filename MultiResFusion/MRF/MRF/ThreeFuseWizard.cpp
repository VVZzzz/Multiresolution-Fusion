#include "ThreeFuseWizard.h"
#include "tools.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QString>

FuseWizard::FuseWizard(QWidget *parent /* = 0 */) : QWizard(parent) {
  intropage = new IntroPage(this);
  porepage = new PorePage(this);
  seriespage = new SeriesPage(this);
  addPage(intropage);
  addPage(porepage);
  addPage(seriespage);

  //设置Banner背景
  //setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
  //setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));
  setWindowTitle(TR("三维融合三维"));
}

void FuseWizard::accept() { QDialog::accept(); }

IntroPage::IntroPage(QWidget *parent /* = 0 */) : QWizardPage(parent) {
  setTitle(TR("三维融合三维向导"));
  //设置Watermark背景
  //setPixmap(QWizard::WatermarkPixmap,QPixmap(":/xxxxx"))
  label = new QLabel(
      TR("\t此向导将帮助您进行融合程序的完成，流程为将多个局部小孔三维体融合入"
         "大孔三维体中。\r\n\t首先我们会计算大孔三维体的孔隙度，"
         "之后需要您提供预计的融合后的"
         "大孔孔隙度，最后需要您提供小孔三维体的数量与路径，"
         "最终生成的三维体在reconstruct"
         "文件夹下。"),this);
  label->setWordWrap(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(label);
  setLayout(layout);
}

PorePage::PorePage(QWidget *parent /* = 0 */) : QWizardPage(parent) {
  setTitle(TR("计算大孔孔隙度"));
  label = new QLabel(TR("计算中..."), this);
  progressbar = new QProgressBar(this);
  progressbar->setRange(0, 100);
  progressbar->setValue(0);
  label2 = new QLabel(TR("孔隙度为: "), this);
  label2->setVisible(false);
  lineedit = new QLineEdit(this);
  lineedit->setVisible(false);
  label3 = new QLabel(TR("输入预计孔隙度: "), this);
  label3->setVisible(false);

  QGridLayout *glayout = new QGridLayout(this);
  glayout->addWidget(label, 0, 0, 1, 2);
  glayout->addWidget(progressbar, 1, 0, 1, 2);
  glayout->addWidget(label2, 2, 0, 1, 2);
  glayout->addWidget(label3, 3, 0);
  glayout->addWidget(lineedit, 3, 1);

  setLayout(glayout);

  //注册必填字段值
  registerField("porenum*", lineedit);
}

void PorePage::SetProgress(int val, double pore) { 
  progressbar->setValue(val); 
  if (val == 100) {
    label2->setText(TR("孔隙度为: %1\%").arg(pore)); 
    label2->setVisible(true);
    lineedit->setVisible(true);
    label3->setVisible(true);
  }
}

SeriesPage::SeriesPage(QWidget *parent /* = 0 */) {
  setTitle(TR("打开小孔序列图"));
  label = new QLabel(
      TR("\t此步骤选取多个小孔三维体,首先您需要输入要融合三维体的个数,"
         "之后点击\"打开文件列表\"按钮,依次选取每个三维体文件路径即可!"),this);
  label->setWordWrap(true);
  label2 = new QLabel(TR("输入要融合三维体个数: "),this);
  corenum = new QLineEdit(this);
  button = new QPushButton(TR("打开文件列表"),this);
  button->setObjectName("openfileBtn");

  QGridLayout *glayout = new QGridLayout(this);
  glayout->addWidget(label, 0, 0,1,2);
  glayout->addWidget(label2, 1, 0);
  glayout->addWidget(corenum, 1, 1);
  glayout->addWidget(button, 2, 0,1,2 ,Qt::AlignCenter);

  setLayout(glayout);
}
