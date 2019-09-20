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

  //����Banner����
  //setPixmap(QWizard::BannerPixmap, QPixmap(":/images/banner.png"));
  //setPixmap(QWizard::BackgroundPixmap, QPixmap(":/images/background.png"));
  setWindowTitle(TR("��ά�ں���ά"));
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
  registerField("poresity*", lineedit);
}

void PorePage::SetProgress(int val, double pore) { 
  progressbar->setValue(val); 
  if (val == 100) {
    label2->setText(TR("��϶��Ϊ: %1 \%").arg(pore)); 
    label2->setVisible(true);
    lineedit->setVisible(true);
    label3->setVisible(true);
  }
}

SeriesPage::SeriesPage(QWidget *parent /* = 0 */) {
  setTitle(TR("��С������ͼ"));
  label = new QLabel(
      TR("\t�˲���ѡȡ���С����ά��,��������Ҫ����Ҫ�ں���ά��ĸ���,"
         "֮����\"���ļ��б�\"��ť,����ѡȡÿ����ά���ļ�·������!"
         "\r\n\tע��,���С����ά��ֱ�����Ҫһ��!"),
      this);
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

  //ע������ֶ�ֵ
  registerField("SmallPoreNum*", corenum);

  connect(this->button, SIGNAL(clicked()), this,
          SLOT(onBtnClicked()));

}

void SeriesPage::onBtnClicked() { 
  qDebug() << "clicked";
  if (corenum->text().isEmpty()) {
    QMessageBox msgbox;
    msgbox.setText(TR("������������!"));
    msgbox.exec();
  }
  int num = corenum->text().toInt();
  int i = 0;
  while (i < num) {
    QString filepath;
    filepath = QFileDialog::getOpenFileName(
        this, TR("�򿪵�%1����ά��").arg(i+1), ".",
        tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
    strvec.push_back(filepath);
    i++;
  }
}

SetSizePage::SetSizePage(QWidget *parent /* = 0 */) {
  
  setTitle(TR("����ģ��ߴ��뱣��·��"));
  label = new QLabel(TR("\t�˲�������ģʽ��ģ��ߴ����ؽ����·��: "
                        "�ϴ�ģ����ȡ����ģʽ�ܹ���׼ȷ����ʾ����϶����̬������"
                        "����ģ��ߴ�����󣬻����Ӽ������������ؽ�Ч�ʡ�"),
      this);
  label->setWordWrap(true);
  label2 = new QLabel(TR("ģ��ߴ�: "),this);
  label2->setWordWrap(true);
  lineedit = new QLineEdit(this);
  lineedit->setText("3");

  label3 = new QLabel(TR("�ؽ����·��: "), this);
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

  //ע������ֶ�ֵ
  registerField("PoreSetSize", lineedit);
  registerField("SavePath*", lineedit2);

  //�����ź����
  connect(this->btn, &QToolButton::clicked, this, &SetSizePage::SetSavePath);
}
void SetSizePage::SetSavePath() {
  QString savepath = QFileDialog::getExistingDirectory(this, TR("����·��"),
                                                       QDir::currentPath());
  if (savepath.isEmpty()) return;
  lineedit2->setText(savepath);
}

FinishPage::FinishPage(QWidget *parent /* = 0 */) {
  label = new QLabel(
      TR("\t����ɲ������ã�����\"Finish\"�����ؽ���"),this);
  label->setWordWrap(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(label);
  setLayout(layout);
}
