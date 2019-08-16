#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  
  initToolButtons();

  ui->setupUi(this);    
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
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 m_threedim2file);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 m_threedim_3);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_singleReconstructOp);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_twoFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 m_threeFuseThreeOP);
  ui->ribbonTabWidget->addButton(TR("工具"), TR("取消当前操作"),
                                 m_cancleCurrOP);

  initListWidget();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::initToolButtons() {
  //在"打开文件"下添加按钮
  //单张三维重建
  m_singleResconstructFile = new QToolButton(this);
  m_singleResconstructFile->setObjectName("singleReFile");
  m_singleResconstructFile->setText(TR("打开单张三维重建"));
  m_singleResconstructFile->setToolTip(TR("打开用来三维重建的单张序列图"));
  m_singleResconstructFile->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_singleResconstructFile->setEnabled(true);

  //二维融合三维
  m_twodim1file = new QToolButton(this);
  m_twodim1file->setObjectName("twodim1");
  m_twodim1file->setText(TR("打开三维重建序列图"));
  m_twodim1file->setToolTip(TR("打开经过单张三维重建得到的序列图"));
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

  m_threedim_3 = new QToolButton(this);
  m_threedim_3->setObjectName("threedim_3");
  m_threedim_3->setText(TR("打开低分辨大孔序列图"));
  m_threedim_3->setToolTip(TR("打开低分辨率大孔的序列图"));
  m_threedim_3->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  m_threedim_3->setEnabled(true);

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
  //m_cancleCurrOP->setText(TR("取消当前操作"));
  m_cancleCurrOP->setToolTip(TR("取消当前操作"));
  m_cancleCurrOP->setIcon(QIcon("./Resources/icons/cancel_1.svg"));
  m_cancleCurrOP->setEnabled(true);

}

void MainWindow::initListWidget() {
  //ui->listWidget->setWindowTitle(TR("图片列表"));
  //ui->listWidget->resize(300, 400);
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
  //imageItem->setSizeHint(QSize(100, 100));
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
  QFileInfoList fileinfolist = dir.entryInfoList();

  //清空m_filesPath
  m_filespath.clear();
  //清除"视图"区域
  clearView();
  //清除"图片列表"区域
  clearListWidget();

  for (auto itr = fileinfolist.begin(); 
    itr != fileinfolist.end(); itr++) {
    //将文件路径添加到m_filespath
    QString tempPath = itr->absoluteFilePath();
    m_filespath.push_back(tempPath);
  //添加缩略图到"列表"区域
    addImage2List(tempPath);
  }

  //添加第一张图片到"视图"区域
  addImage2View(fileinfolist.begin()->absoluteFilePath());
  //显示
  ui->listWidget->show();
  return true;
}

void MainWindow::on_twodim1_clicked() { 
  if (!openSeriesImg()) return;
  //设置图标disable
  disableFileButtons();
  m_twodim1file->setEnabled(true);
  m_singleReconstructOp->setEnabled(false);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
}

void MainWindow::on_threedim1_clicked() {
  if (!openSeriesImg()) return;
  //设置图标disable
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
  //设置图标disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(false);
}
