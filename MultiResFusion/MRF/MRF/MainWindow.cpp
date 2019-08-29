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

  //初始化进度条text
  ui->label->setText(TR("操作进度指示"));
  ui->progressBar->reset();
  ui->progressBar->setRange(0, 100);
  ui->progressBar->setVisible(false);

  //初始化diasble toolsbutton
  m_singleReconstructOp->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);

  //连接信号和槽
  connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this,
          SLOT(listItem_clicked(QListWidgetItem *)));

  //初始化状态栏
  m_sizeLabel = new QLabel(this);
  ui->statusBar->addPermanentWidget(m_sizeLabel);

  //设置窗口标题
  this->setWindowTitle(TR("多分辨率融合软件"));

  //设置"场景"
  m_graphScene = new QGraphicsScene(this);
  m_graphScene->setBackgroundBrush(QColor::fromRgb(224, 224, 224));
  ui->graphicsView->setScene(m_graphScene);

  //隐藏ribbon dock bar
  ui->ribbonDockWidget->setTitleBarWidget(new QWidget());

  //在ribbon上添加tabs
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/briefcase_1.svg"),
                              TR("打开文件"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/add_database_2.svg"),
                              TR("融合操作"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/engineering_1.svg"),
                              TR("工具"));
  ui->ribbonTabWidget->addTab(QIcon("./Resources/icons/information_1.svg"),
                              TR("帮助"));

  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("单张重建"),
                                 m_singleResconstructFile);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("二维融合三维"),
                                 m_twodim1file);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("二维融合三维"),
                                 m_threedim1file);
  /*
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 m_threedim2file);
  */

  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 m_threedim3file);

  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_singleReconstructOp);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_twoFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_threeFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("工具"), TR("取消当前操作"),
                                 m_cancleCurrOP);
  initListWidget();

  m_pCAnneal = nullptr;
  m_pPoreset = nullptr;

  //为自定义对话框分配空间
  m_singleReconDlg = new SingleReconDialog(this);
  m_twoFuseDlg = new TwoFuseDlg(this);
  m_pFuseWizard = new FuseWizard(this);

  //初始化线程对象
  m_pWorkthread = new WorkThread();
  //连接工作线程对象信号和槽
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
  //在"打开文件"下添加按钮
  //单张三维重建
  m_singleResconstructFile = new QToolButton(this);
  m_singleResconstructFile->setObjectName("singleReFile");
  m_singleResconstructFile->setText(TR("打开单张二维图片"));
  m_singleResconstructFile->setToolTip(TR("打开用来三维重建的单张图"));
  m_singleResconstructFile->setIcon(
      QIcon("./Resources/icons/add_folder_2.svg"));
  m_singleResconstructFile->setEnabled(true);

  //二维融合三维
  m_twodim1file = new QToolButton(this);
  m_twodim1file->setObjectName("twodim1");
  m_twodim1file->setText(TR("打开单张二维图片"));
  m_twodim1file->setToolTip(TR("打开要融合的二维图片"));
  m_twodim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_twodim1file->setEnabled(true);

  m_threedim1file = new QToolButton(this);
  m_threedim1file->setObjectName("threedim1");
  m_threedim1file->setText(TR("打开低分辨大孔序列图"));
  m_threedim1file->setToolTip(TR("打开低分辨率大孔的序列图"));
  m_threedim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim1file->setEnabled(true);

  //三维融合三维
  m_threedim2file = new QToolButton(this);
  m_threedim2file->setObjectName("threedim2");
  m_threedim2file->setObjectName("threedim2");
  m_threedim2file->setText(TR("打开高分辨小孔序列图"));
  m_threedim2file->setToolTip(TR("打开局部小孔高分辨率序列图"));
  m_threedim2file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim2file->setEnabled(true);

  m_threedim3file = new QToolButton(this);
  m_threedim3file->setObjectName("threedim3");
  m_threedim3file->setText(TR("打开低分辨大孔序列图"));
  m_threedim3file->setToolTip(TR("打开低分辨率大孔的序列图"));
  m_threedim3file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim3file->setEnabled(true);

  //在"融合操作"下添加按钮
  m_singleReconstructOp = new QToolButton(this);
  m_singleReconstructOp->setObjectName("singleReOP");
  m_singleReconstructOp->setText(TR("三维重建"));
  m_singleReconstructOp->setToolTip(TR("由单张图进行三维重建"));
  m_singleReconstructOp->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_singleReconstructOp->setEnabled(true);

  m_twoFuseThreeOP = new QToolButton(this);
  m_twoFuseThreeOP->setObjectName("twoFuseOP");
  m_twoFuseThreeOP->setText(TR("二维融合三维"));
  m_twoFuseThreeOP->setToolTip(TR("将二维重建融合进三维"));
  m_twoFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_twoFuseThreeOP->setEnabled(true);

  m_threeFuseThreeOP = new QToolButton(this);
  m_threeFuseThreeOP->setObjectName("threeFuseOP");
  m_threeFuseThreeOP->setText(TR("三维融合三维"));
  m_threeFuseThreeOP->setToolTip(TR("将三维融合进三维"));
  m_threeFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  m_threeFuseThreeOP->setEnabled(true);

  //"工具"组按钮
  m_cancleCurrOP = new QToolButton(this);
  m_cancleCurrOP->setObjectName("cancleCurrOP");
  // m_cancleCurrOP->setText(TR("取消当前操作"));
  m_cancleCurrOP->setToolTip(TR("取消当前操作"));
  m_cancleCurrOP->setIcon(QIcon("./Resources/icons/cancel_1.svg"));
  m_cancleCurrOP->setEnabled(true);
}

void MainWindow::initListWidget() {
  // ui->listWidget->setWindowTitle(TR("图片列表"));
  // ui->listWidget->resize(300, 400);
  //设置QListWidget的显示模式
  ui->listWidget->setViewMode(QListView::IconMode);
  //设置QListWidget中单元项的图片大小
  ui->listWidget->setIconSize(QSize(100, 80));
  //设置QListWidget中单元项的间距
  ui->listWidget->setSpacing(10);
  //设置自动适应布局调整（Adjust适应，Fixed不适应），默认不适应
  ui->listWidget->setResizeMode(QListWidget::Adjust);
  //设置不能移动
  ui->listWidget->setMovement(QListWidget::Snap);

  ui->listWidget->addItem(TR("图片列表"));
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
  ui->listWidget->addItem(TR("图片列表"));
}

void MainWindow::addImage2View(const QString &filepath) {
  m_pixmap = QPixmap(filepath);
  m_graphScene->addPixmap(m_pixmap);
  m_graphScene->setSceneRect(QRectF(m_pixmap.rect()));
  setWindowTitle(QFileInfo(filepath).fileName() + "-" + TR("多分辨率融合软件"));
  m_sizeLabel->setText(QString::number(m_pixmap.width()) + "*" +
                       QString::number(m_pixmap.width()));
}

void MainWindow::addImage2List(const QString &filepath) {
  QListWidgetItem *imageItem = new QListWidgetItem;
  imageItem->setIcon(QIcon(filepath));
  int i = m_filespath.size();
  imageItem->setText(QFileInfo(filepath).fileName() + "-" + QString::number(i));
  imageItem->setTextAlignment(Qt::AlignHCenter);
  //不要下面这个设置语句了
  // imageItem->setSizeHint(QSize(100, 100));
  ui->listWidget->addItem(imageItem);
  //注意这个imageItem无需手动清除,listWidget中的clear方法会delete掉所有item.
}

/**
 *   打开单张图片并更新View,List
 */
bool MainWindow::openSingleImg() {
  QString filepath;

  filepath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (filepath.isEmpty()) return false;
  //清空m_filesPath
  m_filespath.clear();

  //将filepath添加至m_filespath中
  m_filespath.push_back(filepath);

  //清除"视图"区域
  clearView();
  //清除"图片列表"区域
  clearListWidget();

  //添加图片到"视图"区域
  addImage2View(filepath);
  //添加缩略图到"列表"区域
  addImage2List(filepath);
  //显示
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

  //清空m_filesPath
  m_filespath.clear();
  //清除"视图"区域
  clearView();
  //清除"图片列表"区域
  clearListWidget();

  int total = m_fileinfolist.size();
  int curr = 1;
  for (auto itr = m_fileinfolist.begin(); itr != m_fileinfolist.end();
       itr++, curr++) {
    //将文件路径添加到m_filespath
    QString tempPath = itr->absoluteFilePath();
    m_filespath.push_back(tempPath);
    //添加缩略图到"列表"区域
    addImage2List(tempPath);
    ui->progressBar->setValue(curr * 100 / total);
  }

  //添加第一张图片到"视图"区域
  addImage2View(m_fileinfolist.begin()->absoluteFilePath());
  //显示
  ui->listWidget->show();
  return true;
}

void MainWindow::on_twodim1_clicked() {
  //设置读入类型
  m_imgtype = OPType::SINGLE;
  //设置进度条
  ui->label->setText(TR("打开图片"));

  if (!openSingleImg()) return;
  ui->progressBar->setVisible(true);
  ui->progressBar->setValue(100);
  //设置图标disable
  disableFileButtons();
  m_twodim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);

  ui->label->setText(TR("操作完成!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim1_clicked() {
  //设置读入类型
  m_imgtype = OPType::LOW2SERIES;
  //设置进度条
  ui->label->setText(TR("打开图片"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //设置图标disable
  disableFileButtons();
  m_threedim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
  ui->label->setText(TR("操作完成!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim2_clicked() {
  //设置读入类型
  m_imgtype = OPType::HIGH3SERIES;
  //设置进度条
  ui->label->setText(TR("打开图片"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //设置图标disable
  disableFileButtons();
  m_threedim2file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(true);
  ui->label->setText(TR("操作完成!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_threedim3_clicked() {
  //设置读入类型
  m_imgtype = OPType::LOW3SERIES;
  //设置进度条
  ui->label->setText(TR("打开图片"));
  ui->progressBar->setVisible(true);

  if (!openSeriesImg()) return;
  //设置图标disable
  disableFileButtons();
  m_threedim3file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(true);
  ui->label->setText(TR("操作完成!"));
  ui->progressBar->setVisible(false);
}

void MainWindow::on_cancleCurrOP_clicked() {
  if (m_pCAnneal) {
    //取消在工作线程中正在执行的操作
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
    msgbox.setText(TR("请重新打开处理图片!"));
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
    msgbox.setText(TR("操作取消!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }

  if (m_pCAnneal) delete m_pCAnneal;
  //在这里申请内存,在线程取消或者完成时,会直接delete
  m_pCAnneal = new CAnnealing(m_filespath[0], finalsz, 3);
  //连接三维重建操作进度信号和槽
  connect(m_pCAnneal, &CAnnealing::CurrProgress, this,
          &MainWindow::on_progress);

  m_pCAnneal->SetSavePath(savepath);
  m_pCAnneal->SetReconOP();
  m_pWorkthread->setAnnealPtr(m_pCAnneal);
  m_pWorkthread->start();
  // m_pCAnneal->Reconstruct();
  //使用完就delete掉
  // delete m_pCAnneal;
  // m_pCAnneal = nullptr;
  //设置进度条
  ui->label->setText(TR("重建操作:"));
  ui->progressBar->reset();
  ui->progressBar->setVisible(true);
}

void MainWindow::on_twoFuseOP_clicked() {
  if (m_imgtype == EMPTY) {
    QMessageBox msgbox;
    msgbox.setText(TR("请重新打开处理图片!"));
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
    msgbox.setText(TR("操作取消!"));
    msgbox.exec();
    m_twoFuseThreeOP->setDown(false);
    return;
  }

  //已打开单张
  if (m_imgtype == OPType::SINGLE) {
    finalsz = QImage(fileinfolist.begin()->absoluteFilePath()).width();
    if (finalsz < m_pixmap.width()) {
      QMessageBox msgbox;
      msgbox.setText(TR("序列图尺寸小于已打开图片!"));
      msgbox.exec();
      m_twoFuseThreeOP->setDown(false);
      return;
    }
    m_pCAnneal = new CAnnealing(m_filespath[0], finalsz, 3);
    // m_pCAnneal->Load3DImg(fileinfolist);
  }
  //已打开序列图
  else if (m_imgtype == OPType::LOW2SERIES) {
    finalsz = m_pixmap.width();
    if (finalsz < QImage(imgpath).width()) {
      QMessageBox msgbox;
      msgbox.setText(TR("序列图尺寸小于打开图片!"));
      msgbox.exec();
      m_twoFuseThreeOP->setDown(false);
      return;
    }
    fileinfolist = m_fileinfolist;
    m_pCAnneal = new CAnnealing(imgpath, finalsz, 3);
  }

  //连接三维重建操作进度信号和槽
  connect(m_pCAnneal, &CAnnealing::CurrProgress, this,
          &MainWindow::on_progress);

  m_pCAnneal->Load3DImg(fileinfolist);
  m_pCAnneal->SetSavePath(savepath);
  m_pCAnneal->SetFuseOP();

  m_pWorkthread->setAnnealPtr(m_pCAnneal);
  m_pWorkthread->start();

  ui->label->setText(TR("融合操作:"));
  ui->progressBar->reset();
  ui->progressBar->setVisible(true);
}

void MainWindow::on_threeFuseOP_clicked() {
  if (m_imgtype == EMPTY) {
    QMessageBox msgbox;
    msgbox.setText(TR("请重新打开处理图片!"));
    msgbox.exec();
    m_singleReconstructOp->setDown(false);
    return;
  }
  m_singleReconstructOp->setDown(false);
  m_twoFuseThreeOP->setDown(false);
  m_threeFuseThreeOP->setDown(true);

  /*
  m_pPoreset = new PoreSet(1, 3);
  //step1: 先设置小孔
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
  savepath = QFileDialog::getExistingDirectory(this, TR("保存路径"),
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

  //连接向导页信号
  connect(m_pPoreset, &PoreSet::LoadBigPorePro, this->m_pFuseWizard->porepage,
          &PorePage::SetProgress);

  // step1: 先设置小孔
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
    //解绑这个m_pCAnneal对象的信号
    m_pCAnneal->disconnect();
    delete m_pCAnneal;
    m_pCAnneal = nullptr;
  }
  ui->label->setText(TR("操作完成!"));
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
  ui->label->setText(TR("操作取消!"));
  QMessageBox msgBox;
  msgBox.setText(TR("操作已取消"));
  msgBox.exec();
}

void MainWindow::on_singleReFile_clicked() {
  //设置图片类型
  m_imgtype = OPType::SINGLE;
  //设置进度条
  ui->label->setText(TR("打开图片"));
  if (!openSingleImg()) return;
  ui->progressBar->setVisible(true);
  ui->progressBar->setValue(100);
  //设置图标disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);

  ui->label->setText(TR("操作完成!"));
  ui->progressBar->setVisible(false);
}
