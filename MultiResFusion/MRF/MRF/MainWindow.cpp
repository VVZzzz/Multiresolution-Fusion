#include "MainWindow.h"
#include <QThread>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  initToolButtons();

  ui->setupUi(this);

  //��ʼ��������text
  ui->label->setText(TR("��������ָʾ"));
  ui->progressBar->reset();
  ui->progressBar->setRange(0, 100);
  ui->progressBar->setVisible(false);

  //��ʼ��diasble toolsbutton
  m_singleReconstructOp->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);

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
  /*
  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_threedim2file);
  */

  ui->ribbonTabWidget->addButton(TR("���ļ�"), TR("��ά�ں���ά"),
                                 m_threedim3file);

  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_singleReconstructOp);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_twoFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("�ںϲ���"), TR("�ںϲ���"),
                                 m_threeFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("����"), TR("ȡ����ǰ����"),
                                 m_cancleCurrOP);
  initListWidget();

  m_pCAnneal = nullptr;
  m_pPoreset = nullptr;

  //Ϊ�Զ���Ի������ռ�
  m_singleReconDlg = new SingleReconDialog(this);
  m_twoFuseDlg = new TwoFuseDlg(this);
  m_pFuseWizard = new FuseWizard(this);

  //��ʼ���̶߳���
  m_pWorkthread = new WorkThread();
  //���ӹ����̶߳����źźͲ�
  connect(m_pWorkthread, &QThread::finished, this,
          &MainWindow::on_workthread_finished);
  connect(m_pWorkthread, &WorkThread::opCancle, this,
          &MainWindow::on_canlethread);
}

MainWindow::~MainWindow() {
  delete ui;
  if (m_pCAnneal != nullptr) {
    delete m_pCAnneal;
    m_pCAnneal = nullptr;
  }
  if (m_pPoreset) {
    delete m_pPoreset;
    m_pPoreset = nullptr;
  }
  if (m_pWorkthread) {
    delete m_pWorkthread;
    m_pWorkthread = nullptr;
  }
}

void MainWindow::initToolButtons() {
  //��"���ļ�"����Ӱ�ť
  //������ά�ؽ�
  m_singleResconstructFile = new QToolButton(this);
  m_singleResconstructFile->setObjectName("singleReFile");
  m_singleResconstructFile->setText(TR("�򿪵��Ŷ�άͼƬ"));
  m_singleResconstructFile->setToolTip(TR("��������ά�ؽ��ĵ���ͼ"));
  m_singleResconstructFile->setIcon(
      QIcon("./Resources/icons/add_folder_2.svg"));
  m_singleResconstructFile->setEnabled(true);

  //��ά�ں���ά
  m_twodim1file = new QToolButton(this);
  m_twodim1file->setObjectName("twodim1");
  m_twodim1file->setText(TR("�򿪵��Ŷ�άͼƬ"));
  m_twodim1file->setToolTip(TR("��Ҫ�ںϵĶ�άͼƬ"));
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

  m_threedim3file = new QToolButton(this);
  m_threedim3file->setObjectName("threedim3");
  m_threedim3file->setText(TR("�򿪵ͷֱ�������ͼ"));
  m_threedim3file->setToolTip(TR("�򿪵ͷֱ��ʴ�׵�����ͼ"));
  m_threedim3file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim3file->setEnabled(true);

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
  // m_cancleCurrOP->setText(TR("ȡ����ǰ����"));
  m_cancleCurrOP->setToolTip(TR("ȡ����ǰ����"));
  m_cancleCurrOP->setIcon(QIcon("./Resources/icons/cancel_1.svg"));
  m_cancleCurrOP->setEnabled(true);
}

void MainWindow::initListWidget() {
  // ui->listWidget->setWindowTitle(TR("ͼƬ�б�"));
  // ui->listWidget->resize(300, 400);
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
  m_threedim3file->setEnabled(false);
}

void MainWindow::enableFileButtons() {
  m_singleResconstructFile->setEnabled(true);
  m_twodim1file->setEnabled(true);
  m_threedim1file->setEnabled(true);
  m_threedim2file->setEnabled(true);
  m_threedim3file->setEnabled(true);
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
  // imageItem->setSizeHint(QSize(100, 100));
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
  m_fileinfolist = dir.entryInfoList();

  //���m_filesPath
  m_filespath.clear();
  //���"��ͼ"����
  clearView();
  //���"ͼƬ�б�"����
  clearListWidget();

  int total = m_fileinfolist.size();
  int curr = 1;
  for (auto itr = m_fileinfolist.begin(); itr != m_fileinfolist.end();
       itr++, curr++) {
    //���ļ�·����ӵ�m_filespath
    QString tempPath = itr->absoluteFilePath();
    m_filespath.push_back(tempPath);
    //�������ͼ��"�б�"����
    addImage2List(tempPath);
    ui->progressBar->setValue(curr * 100 / total);
  }

  //��ӵ�һ��ͼƬ��"��ͼ"����
  addImage2View(m_fileinfolist.begin()->absoluteFilePath());
  //��ʾ
  ui->listWidget->show();
  return true;
}

void MainWindow::on_twodim1_clicked() {
  //���ö�������
  m_imgtype = OPType::SINGLE;
  //���ý�����
  ui->label->setText(TR("��ͼƬ"));

  if (!openSingleImg()) return;
  ui->progressBar->setVisible(true);
  ui->progressBar->setValue(100);
  //����ͼ��disable
  disableFileButtons();
  m_twodim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);

  ui->label->setText(TR("�������!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim1_clicked() {
  //���ö�������
  m_imgtype = OPType::LOW2SERIES;
  //���ý�����
  ui->label->setText(TR("��ͼƬ"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_threedim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
  ui->label->setText(TR("�������!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim2_clicked() {
  //���ö�������
  m_imgtype = OPType::HIGH3SERIES;
  //���ý�����
  ui->label->setText(TR("��ͼƬ"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_threedim2file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(true);
  ui->label->setText(TR("�������!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim3_clicked() {
  //���ö�������
  m_imgtype = OPType::LOW3SERIES;
  //���ý�����
  ui->label->setText(TR("��ͼƬ"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //����ͼ��disable
  disableFileButtons();
  m_threedim3file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(true);
  ui->label->setText(TR("�������!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_cancleCurrOP_clicked() {
  if (m_pCAnneal) {
    //ȡ���ڹ����߳�������ִ�еĲ���
    m_pCAnneal->ShutDown();
  }
  enableFileButtons();
  m_singleReconstructOp->setEnabled(true);
  m_singleReconstructOp->setDown(false);

  m_twoFuseThreeOP->setEnabled(true);
  m_twoFuseThreeOP->setDown(false);

  m_threeFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setDown(false);
  m_imgtype = EMPTY;
}

void MainWindow::listItem_clicked(QListWidgetItem *item) {
  // qDebug() << ui->listWidget->row(item);
  int index = ui->listWidget->row(item);
  if (index == 0) return;
  clearView();
  addImage2View(m_filespath[index - 1]);
}

void MainWindow::on_singleReOP_clicked() {
  if (m_imgtype == EMPTY) {
    QMessageBox msgbox;
    msgbox.setText(TR("�����´򿪴���ͼƬ!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }
  m_singleReconstructOp->setDown(true);
  m_twoFuseThreeOP->setDown(false);
  m_threeFuseThreeOP->setDown(false);

  QString savepath;
  int finalsz;
  if (m_singleReconDlg->exec() == QDialog::Accepted) {
    savepath = m_singleReconDlg->getPath();
    finalsz = m_singleReconDlg->getSize();
  } else {
    QMessageBox msgbox;
    msgbox.setText(TR("����ȡ��!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }

  if (m_pCAnneal) delete m_pCAnneal;
  //�����������ڴ�,���߳�ȡ���������ʱ,��ֱ��delete
  m_pCAnneal = new CAnnealing(m_filespath[0], finalsz, 3);
  //������ά�ؽ����������źźͲ�
  connect(m_pCAnneal, &CAnnealing::CurrProgress, this,
          &MainWindow::on_progress);

  m_pCAnneal->SetSavePath(savepath);
  m_pCAnneal->SetReconOP();
  m_pWorkthread->setAnnealPtr(m_pCAnneal);
  m_pWorkthread->start();
  // m_pCAnneal->Reconstruct();
  //ʹ�����delete��
  // delete m_pCAnneal;
  // m_pCAnneal = nullptr;
  //���ý�����
  ui->label->setText(TR("�ؽ�����:"));
  ui->progressBar->reset();
  ui->progressBar->setVisible(true);
}

void MainWindow::on_twoFuseOP_clicked() {
  if (m_imgtype == EMPTY) {
    QMessageBox msgbox;
    msgbox.setText(TR("�����´򿪴���ͼƬ!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }
  m_singleReconstructOp->setDown(false);
  m_twoFuseThreeOP->setDown(true);
  m_threeFuseThreeOP->setDown(false);

  QString imgpath;
  QString savepath;
  QFileInfoList fileinfolist;
  int finalsz;
  if (m_pCAnneal) delete m_pCAnneal;
  if (m_imgtype == OPType::SINGLE)
    m_twoFuseDlg->setType(SINGLE);
  else if (m_imgtype == OPType::LOW2SERIES)
    m_twoFuseDlg->setType(LOW2SERIES);

  if (m_twoFuseDlg->exec() == QDialog::Accepted) {
    imgpath = m_twoFuseDlg->getImgPath();
    savepath = m_twoFuseDlg->getSavePath();
    fileinfolist = m_twoFuseDlg->getInfoList();
  } else {
    QMessageBox msgbox;
    msgbox.setText(TR("����ȡ��!"));
    msgbox.exec();
    m_twoFuseThreeOP->setDown(false);
    return;
  }

  //�Ѵ򿪵���
  if (m_imgtype == OPType::SINGLE) {
    finalsz = QImage(fileinfolist.begin()->absoluteFilePath()).width();
    if (finalsz < m_pixmap.width()) {
      QMessageBox msgbox;
      msgbox.setText(TR("����ͼ�ߴ�С���Ѵ�ͼƬ!"));
      msgbox.exec();
      m_twoFuseThreeOP->setDown(false);
      return;
    }
    m_pCAnneal = new CAnnealing(m_filespath[0], finalsz, 3);
    // m_pCAnneal->Load3DImg(fileinfolist);
  }
  //�Ѵ�����ͼ
  else if (m_imgtype == OPType::LOW2SERIES) {
    finalsz = m_pixmap.width();
    if (finalsz < QImage(imgpath).width()) {
      QMessageBox msgbox;
      msgbox.setText(TR("����ͼ�ߴ�С�ڴ�ͼƬ!"));
      msgbox.exec();
      m_twoFuseThreeOP->setDown(false);
      return;
    }
    fileinfolist = m_fileinfolist;
    m_pCAnneal = new CAnnealing(imgpath, finalsz, 3);
  }

  //������ά�ؽ����������źźͲ�
  connect(m_pCAnneal, &CAnnealing::CurrProgress, this,
          &MainWindow::on_progress);

  m_pCAnneal->Load3DImg(fileinfolist);
  m_pCAnneal->SetSavePath(savepath);
  m_pCAnneal->SetFuseOP();

  m_pWorkthread->setAnnealPtr(m_pCAnneal);
  m_pWorkthread->start();

  ui->label->setText(TR("�ںϲ���:"));
  ui->progressBar->reset();
  ui->progressBar->setVisible(true);
}

void MainWindow::on_threeFuseOP_clicked() {
  if (m_imgtype == EMPTY) {
    QMessageBox msgbox;
    msgbox.setText(TR("�����´򿪴���ͼƬ!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }
  m_singleReconstructOp->setDown(false);
  m_twoFuseThreeOP->setDown(false);
  m_threeFuseThreeOP->setDown(true);

  /*
  m_pPoreset = new PoreSet(1, 3);
  //step1: ������С��
  m_pPoreset->SetSmallSize(128, 128, 128);
  m_pPoreset->SetBigSize(128, 128, 128);
  m_pPoreset->SetExpectPorosity(28.00);

  QString sfilepath;
  sfilepath = QFileDialog::getOpenFileName(
      this, tr("Open Small Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (sfilepath.isEmpty()) return ;

  QString bfilepath;
  bfilepath = QFileDialog::getOpenFileName(
      this, tr("Open Big Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (bfilepath.isEmpty()) return ;

  QString savepath;
  savepath = QFileDialog::getExistingDirectory(this, TR("����·��"),
                                               QDir::currentPath());
  if (savepath.isEmpty()) return ;

  vector<QString> smallpathvec = {sfilepath};
  if (!m_pPoreset->LoadSmallPoreSet(smallpathvec)) {
    qDebug() << "Load SmallPoreSet error!";
    return;
  }
    qDebug() << "Load SmallPoreSet success!";
  if (!m_pPoreset->LoadBigPoreSet(bfilepath)) {
    qDebug() << "Load BigPoreSet error!";
    return;
  }
    qDebug() << "Load BigPoreSet success!";

  if (!m_pPoreset->Reconstruct(savepath)) {
    qDebug() << "Reconstruct error!";
    return;
  }
  delete m_pPoreset;
  */
  m_pPoreset = new PoreSet(1, 3);

  //������ҳ�ź�
  connect(m_pPoreset, &PoreSet::LoadBigPorePro, this->m_pFuseWizard->porepage,
          &PorePage::SetProgress);

  // step1: ������С��
  m_pPoreset->SetSmallSize(128, 128, 128);
  m_pPoreset->SetBigSize(128, 128, 128);
  // m_pPoreset->SetExpectPorosity(28.00);
  m_pFuseWizard->show();
  m_pPoreset->LoadBigPoreSet(m_filespath[0]);

  delete m_pPoreset;
  m_pPoreset = nullptr;
}

void MainWindow::on_workthread_finished() {
  if (m_pCAnneal) {
    //������m_pCAnneal������ź�
    m_pCAnneal->disconnect();
    delete m_pCAnneal;
    m_pCAnneal = nullptr;
  }
  ui->label->setText(TR("�������!"));
  ui->progressBar->setValue(100);
  ui->progressBar->setVisible(false);

  enableFileButtons();
  m_singleReconstructOp->setEnabled(true);
  m_singleReconstructOp->setDown(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_twoFuseThreeOP->setDown(false);
  m_threeFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setDown(false);
  m_imgtype = EMPTY;
}

void MainWindow::on_progress(int val) { ui->progressBar->setValue(val); }

void MainWindow::on_canlethread() {
  ui->progressBar->setVisible(false);
  ui->label->setText(TR("����ȡ��!"));
  QMessageBox msgBox;
  msgBox.setText(TR("������ȡ��"));
  msgBox.exec();
}

void MainWindow::on_singleReFile_clicked() {
  //����ͼƬ����
  m_imgtype = OPType::SINGLE;
  //���ý�����
  ui->label->setText(TR("��ͼƬ"));
  if (!openSingleImg()) return;
  ui->progressBar->setVisible(true);
  ui->progressBar->setValue(100);
  //����ͼ��disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);

  ui->label->setText(TR("�������!"));
  ui->progressBar->setVisible(false);
}
