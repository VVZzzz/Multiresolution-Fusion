#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  
  initToolButtons();

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


  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("�����ؽ�"),
                                m_singleResconstructFile);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_twodim1file);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_threedim1file);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_threedim2file);
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_threedim_3);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_singleReconstructOp);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_twoFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_threeFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("����"), TR("ȡ����ǰ����"),
                                 m_cancleCurrOP);

  initListWidget();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initToolButtons() {
  //��"���ļ�"����Ӱ�ť
  //������ά�ؽ�
  m_singleResconstructFile = new QToolButton(this);
  m_singleResconstructFile->setObjectName("singleReFile");
  m_singleResconstructFile->setText(TR("�򿪵�����ά�ؽ�"));
  m_singleResconstructFile->setToolTip(TR("��������ά�ؽ��ĵ�������ͼ"));
  m_singleResconstructFile->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_singleResconstructFile->setEnabled(true);

  //��ά�ں���ά
  m_twodim1file = new QToolButton(this);
  m_twodim1file->setObjectName("twodim1");
  m_twodim1file->setText(TR("����ά�ؽ�����ͼ"));
  m_twodim1file->setToolTip(TR("�򿪾���������ά�ؽ��õ�������ͼ"));
  m_twodim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_twodim1file->setEnabled(true);

  m_threedim1file = new QToolButton(this);
  m_threedim1file->setObjectName("threedim1");
  m_threedim1file->setText(TR("�򿪵ͷֱ�������ͼ"));
  m_threedim1file->setToolTip(TR("�򿪵ͷֱ��ʴ�׵�����ͼ"));
  m_threedim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim1file->setEnabled(true);

  //��ά�ں���ά
  m_threedim2file = new QToolButton(this);
  m_threedim2file->setObjectName("threedim2");
  m_threedim2file->setObjectName("threedim2");
  m_threedim2file->setText(TR("�򿪸߷ֱ�С������ͼ"));
  m_threedim2file->setToolTip(TR("�򿪾ֲ�С�׸߷ֱ�������ͼ"));
  m_threedim2file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim2file->setEnabled(true);

  m_threedim_3 = new QToolButton(this);
  m_threedim_3->setObjectName("threedim_3");
  m_threedim_3->setText(TR("�򿪵ͷֱ�������ͼ"));
  m_threedim_3->setToolTip(TR("�򿪵ͷֱ��ʴ�׵�����ͼ"));
  m_threedim_3->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim_3->setEnabled(true);

  //��"�ںϲ���"����Ӱ�ť
  m_singleReconstructOp = new QToolButton(this);
  m_singleReconstructOp->setObjectName("singleReOP");
  m_singleReconstructOp->setText(TR("��ά�ؽ�"));
  m_singleReconstructOp->setToolTip(TR("�ɵ���ͼ������ά�ؽ�"));
  m_singleReconstructOp->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_singleReconstructOp->setEnabled(true);

  m_twoFuseThreeOP = new QToolButton(this);
  m_twoFuseThreeOP->setObjectName("twoFuseOP");
  m_twoFuseThreeOP->setText(TR("��ά�ں���ά"));
  m_twoFuseThreeOP->setToolTip(TR("����ά�ؽ��ںϽ���ά"));
  m_twoFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_twoFuseThreeOP->setEnabled(true);

  m_threeFuseThreeOP = new QToolButton(this);
  m_threeFuseThreeOP->setObjectName("threeFuseOP");
  m_threeFuseThreeOP->setText(TR("��ά�ں���ά"));
  m_threeFuseThreeOP->setToolTip(TR("����ά�ںϽ���ά"));
  m_threeFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_threeFuseThreeOP->setEnabled(true);

  //"����"�鰴ť
  m_cancleCurrOP = new QToolButton(this);
  m_cancleCurrOP->setObjectName("cancleCurrOP");
  //m_cancleCurrOP->setText(TR("ȡ����ǰ����"));
  m_cancleCurrOP->setToolTip(TR("ȡ����ǰ����"));
  m_cancleCurrOP->setIcon(QIcon("./Resources/icons/cancel_1.svg"));
  m_cancleCurrOP->setEnabled(true);

}

void MainWindow::initListWidget() {
  //ui->listWidget->setWindowTitle(TR("ͼƬ�б�"));
  ui->listWidget->resize(365, 400);
  //����QListWidget����ʾģʽ
  ui->listWidget->setViewMode(QListView::IconMode);
  //����QListWidget�е�Ԫ���ͼƬ��С
  ui->listWidget->setIconSize(QSize(100, 100));
  //����QListWidget�е�Ԫ��ļ��
  ui->listWidget->setSpacing(10);
  //�����Զ���Ӧ���ֵ�����Adjust��Ӧ��Fixed����Ӧ����Ĭ�ϲ���Ӧ
  ui->listWidget->setResizeMode(QListWidget::Adjust);
  //���ò����ƶ�
  ui->listWidget->setMovement(QListWidget::Static);

  //QListWidgetItem *listTitle = new QListWidgetItem(ui->listWidget);
  //listTitle->setIcon(QIcon());
  //listTitle->setText(TR("ͼƬ�б�"));

  //ui->listWidget->addItem(listTitle);
  ui->listWidget->addItem(TR("ͼƬ�б�"));
  ui->listWidget->show();

}

void MainWindow::disableFileButtons() {
  m_singleResconstructFile->setEnabled(false);
  m_twodim1file->setEnabled(false);
  m_threedim1file->setEnabled(false);
  m_threedim2file->setEnabled(false);
  m_threedim_3->setEnabled(false);
}

void MainWindow::enableFileButtons() {
  m_singleResconstructFile->setEnabled(true);
  m_twodim1file->setEnabled(true);
  m_threedim1file->setEnabled(true);
  m_threedim2file->setEnabled(true);
  m_threedim_3->setEnabled(true);
}

void MainWindow::on_cancleCurrOP_clicked() { 
  enableFileButtons(); 
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(true);
}

void MainWindow::on_singleReFile_clicked() { 
  //����ͼ��disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);
  //��ͼƬ
  QString filename;
  QFileDialog *fileDlg = new QFileDialog(this);
  fileDlg->setDirectory(".");
  fileDlg->setNameFilter(tr("Images(*.png *.jpg *jpeg *bmp)"));
  fileDlg->setViewMode(QFileDialog::Detail);
  fileDlg->setFileMode(QFileDialog::ExistingFile);

  if (!fileDlg->exec()) return;
  filename = QFileDialog::getOpenFileName(this,
    tr("Open Image"), ".", tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (filename.isEmpty()) return;
}
