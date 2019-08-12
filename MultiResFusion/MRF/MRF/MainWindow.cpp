#include "MainWindow.h"
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  ui->setupUi(this);

  //����ribbon dock bar
  ui->ribbonDockWidget->setTitleBarWidget(new QWidget());

  //��ribbon�����tabs
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/briefcase_1.svg"),
                              TR("���ļ�"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/add_database_2.svg"),
                              TR("�ںϲ���"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/engineering_1.svg"),
                              TR("����"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/information_1.svg"),
                              TR("����"));

  //��"���ļ�"����Ӱ�ť
  //������ά�ؽ�
  QToolButton *singleResconstructFile = new QToolButton(this);
  singleResconstructFile->setText(TR("�򿪵�����ά�ؽ�"));
  singleResconstructFile->setToolTip(TR("��������ά�ؽ��ĵ�������ͼ"));
  singleResconstructFile->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  singleResconstructFile->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("�����ؽ�"),
                                 singleResconstructFile);

  //��ά�ں���ά
  QToolButton *twodim1file = new QToolButton(this);
  twodim1file->setText(TR("����ά�ؽ�����ͼ"));
  twodim1file->setToolTip(TR("�򿪾���������ά�ؽ��õ�������ͼ"));
  twodim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  twodim1file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 twodim1file);

  QToolButton *threedim1file = new QToolButton(this);
  threedim1file->setText(TR("�򿪵ͷֱ�������ͼ"));
  threedim1file->setToolTip(TR("�򿪵ͷֱ��ʴ�׵�����ͼ"));
  threedim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim1file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 threedim1file);

  //��ά�ں���ά
  QToolButton *threedim2file = new QToolButton(this);
  threedim2file->setText(TR("�򿪸߷ֱ�С������ͼ"));
  threedim2file->setToolTip(TR("�򿪾ֲ�С�׸߷ֱ�������ͼ"));
  threedim2file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim2file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 threedim2file);

  QToolButton *threedim_3 = new QToolButton(this);
  threedim_3->setText(TR("�򿪵ͷֱ�������ͼ"));
  threedim_3->setToolTip(TR("�򿪵ͷֱ��ʴ�׵�����ͼ"));
  threedim_3->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim_3->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 threedim_3);
  

  //��"�ںϲ���"����Ӱ�ť
  QToolButton *singleReconstructOp = new QToolButton(this);
  singleReconstructOp->setText(TR("��ά�ؽ�"));
  singleReconstructOp->setToolTip(TR("�ɵ���ͼ������ά�ؽ�"));
  singleReconstructOp->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  singleReconstructOp->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 singleReconstructOp);

  QToolButton *twoFuseThreeOP= new QToolButton(this);
  twoFuseThreeOP->setText(TR("��ά�ں���ά"));
  twoFuseThreeOP->setToolTip(TR("����ά�ؽ��ںϽ���ά"));
  twoFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  twoFuseThreeOP->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 twoFuseThreeOP);
  QToolButton *threeFuseThreeOP= new QToolButton(this);
  threeFuseThreeOP->setText(TR("��ά�ں���ά"));
  threeFuseThreeOP->setToolTip(TR("����ά�ںϽ���ά"));
  threeFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  threeFuseThreeOP->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 threeFuseThreeOP);
}

MainWindow::~MainWindow() { delete ui; }
