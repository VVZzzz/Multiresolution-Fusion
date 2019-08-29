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

  //����Banner����
  //setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
  //setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));
  setWindowTitle(TR("��ά�ں���ά"));
}

void FuseWizard::accept() { QDialog::accept(); }

IntroPage::IntroPage(QWidget *parent /* = 0 */) : QWizardPage(parent) {
  setTitle(TR("��ά�ں���ά��"));
  //����Watermark����
  //setPixmap(QWizard::WatermarkPixmap,QPixmap(":/xxxxx"))
  label = new QLabel(
      TR("\t���򵼽������������ںϳ������ɣ�����Ϊ������ֲ�С����ά���ں���"
         "�����ά���С�\r\n\t�������ǻ��������ά��Ŀ�϶�ȣ�"
         "֮����Ҫ���ṩԤ�Ƶ��ںϺ��"
         "��׿�϶�ȣ������Ҫ���ṩС����ά���������·����"
         "�������ɵ���ά����reconstruct"
         "�ļ����¡�"),this);
  label->setWordWrap(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(label);
  setLayout(layout);
}

PorePage::PorePage(QWidget *parent /* = 0 */) : QWizardPage(parent) {
  setTitle(TR("�����׿�϶��"));
  label = new QLabel(TR("������..."), this);
  progressbar = new QProgressBar(this);
  progressbar->setRange(0, 100);
  progressbar->setValue(0);
  label2 = new QLabel(TR("��϶��Ϊ: "), this);
  label2->setVisible(false);
  lineedit = new QLineEdit(this);
  lineedit->setVisible(false);
  label3 = new QLabel(TR("����Ԥ�ƿ�϶��: "), this);
  label3->setVisible(false);

  QGridLayout *glayout = new QGridLayout(this);
  glayout->addWidget(label, 0, 0, 1, 2);
  glayout->addWidget(progressbar, 1, 0, 1, 2);
  glayout->addWidget(label2, 2, 0, 1, 2);
  glayout->addWidget(label3, 3, 0);
  glayout->addWidget(lineedit, 3, 1);

  setLayout(glayout);

  //ע������ֶ�ֵ
  registerField("porenum*", lineedit);
}

void PorePage::SetProgress(int val, double pore) { 
  progressbar->setValue(val); 
  if (val == 100) {
    label2->setText(TR("��϶��Ϊ: %1\%").arg(pore)); 
    label2->setVisible(true);
    lineedit->setVisible(true);
    label3->setVisible(true);
  }
}

SeriesPage::SeriesPage(QWidget *parent /* = 0 */) {
  setTitle(TR("��С������ͼ"));
  label = new QLabel(
      TR("\t�˲���ѡȡ���С����ά��,��������Ҫ����Ҫ�ں���ά��ĸ���,"
         "֮����\"���ļ��б�\"��ť,����ѡȡÿ����ά���ļ�·������!"),this);
  label->setWordWrap(true);
  label2 = new QLabel(TR("����Ҫ�ں���ά�����: "),this);
  corenum = new QLineEdit(this);
  button = new QPushButton(TR("���ļ��б�"),this);
  button->setObjectName("openfileBtn");

  QGridLayout *glayout = new QGridLayout(this);
  glayout->addWidget(label, 0, 0,1,2);
  glayout->addWidget(label2, 1, 0);
  glayout->addWidget(corenum, 1, 1);
  glayout->addWidget(button, 2, 0,1,2 ,Qt::AlignCenter);

  setLayout(glayout);
}
