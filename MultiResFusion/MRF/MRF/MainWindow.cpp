#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  
  initToolButtons();

  ui->setupUi(this);    
  //�����źźͲ�
  connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this,
          SLOT(listItem_clicked(QListWidgetItem *)));

  //��ʼ��״̬��
  m_sizeLabel = new QLabel(this);
  ui->statusBar->addPermanentWidget(m_sizeLabel);

  //���ô��ڱ���
  this->setWindowTitle(TR("��ֱ����ں����"));

  //����"����"
  m_graphScene = new QGraphicsScene(this);
  m_graphScene->setBackgroundBrush(QColor::fromRgb(224, 224, 224));
  ui->graphicsView->setScene(m_graphScene);

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
  //ui->listWidget->resize(300, 400);
  //����QListWidget����ʾģʽ
  ui->listWidget->setViewMode(QListView::IconMode);
  //����QListWidget�е�Ԫ���ͼƬ��С
  ui->listWidget->setIconSize(QSize(100, 80));
  //����QListWidget�е�Ԫ��ļ��
  ui->listWidget->setSpacing(10);
  //�����Զ���Ӧ���ֵ�����Adjust��Ӧ��Fixed����Ӧ����Ĭ�ϲ���Ӧ
  ui->listWidget->setResizeMode(QListWidget::Adjust);
  //���ò����ƶ�
  ui->listWidget->setMovement(QListWidget::Snap);

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

void MainWindow::clearView() { 
  m_graphScene->clear(); 
  ui->graphicsView->resetTransform();
  m_sizeLabel->clear();
}

void MainWindow::clearListWidget() { 
  ui->listWidget->clear(); 
  ui->listWidget->addItem(TR("ͼƬ�б�"));
}

void MainWindow::addImage2View(const QString &filepath) {
  m_pixmap = QPixmap(filepath);
  m_graphScene->addPixmap(m_pixmap);
  m_graphScene->setSceneRect(QRectF(m_pixmap.rect()));
  setWindowTitle(QFileInfo(filepath).fileName() + "-" + TR("��ֱ����ں����"));
  m_sizeLabel->setText(QString::number(m_pixmap.width()) + "*" +
                       QString::number(m_pixmap.width()));
}

void MainWindow::addImage2List(const QString &filepath) {
  QListWidgetItem *imageItem = new QListWidgetItem;
  imageItem->setIcon(QIcon(filepath));
  int i = m_filespath.size();
  imageItem->setText(QFileInfo(filepath).fileName() + "-" + QString::number(i));
  imageItem->setTextAlignment(Qt::AlignHCenter);
  //��Ҫ����������������
  //imageItem->setSizeHint(QSize(100, 100));
  ui->listWidget->addItem(imageItem);
  //ע�����imageItem�����ֶ����,listWidget�е�clear������delete������item.
}


/**
 *   �򿪵���ͼƬ������View,List
 */
bool MainWindow::openSingleImg() {
  QString filepath;

  filepath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (filepath.isEmpty()) return false;
  //���m_filesPath
  m_filespath.clear();

  //��filepath�����m_filespath��
  m_filespath.push_back(filepath);

  //���"��ͼ"����
  clearView();
  //���"ͼƬ�б�"����
  clearListWidget();

  //���ͼƬ��"��ͼ"����
  addImage2View(filepath);
  //�������ͼ��"�б�"����
  addImage2List(filepath);
  //��ʾ
  ui->listWidget->show();
  return true;
}

bool MainWindow::openSeriesImg() {
  QString filepath;
  filepath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (filepath.isEmpty()) return false;

  QDir dir(QFileInfo(filepath).absoluteDir());
  QStringList filters;
  filters << "*.png"
          << "*.jpg"
          << "*.bmp"
          << "*.jpeg";
  dir.setNameFilters(filters);
  QFileInfoList fileinfolist = dir.entryInfoList();

  //���m_filesPath
  m_filespath.clear();
  //���"��ͼ"����
  clearView();
  //���"ͼƬ�б�"����
  clearListWidget();

  for (auto itr = fileinfolist.begin(); 
    itr != fileinfolist.end(); itr++) {
    //���ļ�·����ӵ�m_filespath
    QString tempPath = itr->absoluteFilePath();
    m_filespath.push_back(tempPath);
  //�������ͼ��"�б�"����
    addImage2List(tempPath);
  }

  //��ӵ�һ��ͼƬ��"��ͼ"����
  addImage2View(fileinfolist.begin()->absoluteFilePath());
  //��ʾ
  ui->listWidget->show();
  return true;
}

void MainWindow::on_twodim1_clicked() { 
  if (!openSeriesImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_twodim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
}

void MainWindow::on_threedim1_clicked() {
  if (!openSeriesImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_threedim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
}

void MainWindow::on_cancleCurrOP_clicked() { 
  enableFileButtons(); 
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(true);
}

void MainWindow::listItem_clicked(QListWidgetItem *item) { 
  //qDebug() << ui->listWidget->row(item);
  int index = ui->listWidget->row(item);
  if (index == 0) return;
  clearView();
  addImage2View(m_filespath[index-1]);
}

void MainWindow::on_singleReFile_clicked() {
    if (!openSingleImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
}
