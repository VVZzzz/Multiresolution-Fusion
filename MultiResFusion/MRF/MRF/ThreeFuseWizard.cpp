#include "ThreeFuseWizard.h"
#include "tools.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDir>

FuseWizard::FuseWizard(QWidget *parent /* = 0 */) : QWizard(parent) {
  intropage = new IntroPage(this);
  porepage = new PorePage(this);
  seriespage = new SeriesPage(this);
  setsizepage = new SetSizePage(this);
  finishpage = new FinishPage(this);
  addPage(intropage);
  addPage(porepage);
  addPage(seriespage);
  addPage(setsizepage);
  addPage(finishpage);

  //设置Banner背景
  //setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
  //setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));
  setWindowTitle(TR("三维融合三维"));
}

void FuseWizard::accept() { 
  para.expectPoresity = field("poresity").toDouble();
  para.poresetSize = field("PoreSetSize").toInt();
  para.smallcorenum = field("SmallPoreNum").toInt();
  para.smallPathVec = seriespage->getStrvec();
  para.savepath = field("SavePath").toString();
  QDialog::accept(); 
  emit SetParaFinish();
}

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
  registerField("poresity*", lineedit);
}

void PorePage::SetProgress(int val, double pore) { 
  progressbar->setValue(val); 
  if (val == 100) {
    label2->setText(TR("孔隙度为: %1 \%").arg(pore)); 
    label2->setVisible(true);
    lineedit->setVisible(true);
    label3->setVisible(true);
  }
}

SeriesPage::SeriesPage(QWidget *parent /* = 0 */) {
  setTitle(TR("打开小孔序列图"));
  label = new QLabel(
      TR("\t此步骤选取多个小孔三维体,首先您需要输入要融合三维体的个数,"
         "之后点击\"打开文件列表\"按钮,依次选取每个三维体文件路径即可!"
         "\r\n\t注意,多个小孔三维体分辨率需要一致!"),
      this);
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

  //注册必填字段值
  registerField("SmallPoreNum*", corenum);

  connect(this->button, SIGNAL(clicked()), this,
          SLOT(onBtnClicked()));

}

void SeriesPage::onBtnClicked() { 
  qDebug() << "clicked";
  if (corenum->text().isEmpty()) {
    QMessageBox msgbox;
    msgbox.setText(TR("请先输入数量!"));
    msgbox.exec();
  }
  int num = corenum->text().toInt();
  int i = 0;
  while (i < num) {
    QString filepath;
    filepath = QFileDialog::getOpenFileName(
        this, TR("打开第%1个三维体").arg(i+1), ".",
        tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
    strvec.push_back(filepath);
    i++;
  }
}

SetSizePage::SetSizePage(QWidget *parent /* = 0 */) {
  
  setTitle(TR("设置模板尺寸与保存路径"));
  label = new QLabel(TR("\t此步骤设置模式集模板尺寸与重建结果路径: "
                        "较大模板提取到的模式能够更准确地显示出孔隙的形态特征。"
                        "但是模板尺寸的增大，会增加计算量，降低重建效率。"),
      this);
  label->setWordWrap(true);
  label2 = new QLabel(TR("模板尺寸: "),this);
  label2->setWordWrap(true);
  lineedit = new QLineEdit(this);
  lineedit->setText("3");

  label3 = new QLabel(TR("重建结果路径: "), this);
  label3->setWordWrap(true);
  lineedit2 = new QLineEdit(this);
  btn = new QToolButton(this);
  btn->setText(tr("..."));

  QGridLayout *glayout = new QGridLayout(this);
  glayout->addWidget(label, 0, 0,1,3);
  glayout->addWidget(label2, 1, 0, 1, 1);
  glayout->addWidget(lineedit, 1, 1, 1, 2);
  glayout->addWidget(label3,2,0);
  glayout->addWidget(lineedit2, 2, 1);
  glayout->addWidget(btn, 2, 2);

  setLayout(glayout);

  //注册必填字段值
  registerField("PoreSetSize", lineedit);
  registerField("SavePath*", lineedit2);

  //设置信号与槽
  connect(this->btn, &QToolButton::clicked, this, &SetSizePage::SetSavePath);
}
void SetSizePage::SetSavePath() {
  QString savepath = QFileDialog::getExistingDirectory(this, TR("保存路径"),
                                                       QDir::currentPath());
  if (savepath.isEmpty()) return;
  lineedit2->setText(savepath);
}

FinishPage::FinishPage(QWidget *parent /* = 0 */) {
  label = new QLabel(
      TR("\t已完成参数设置，请点击\"Finish\"进行重建！"),this);
  label->setWordWrap(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(label);
  setLayout(layout);
}
