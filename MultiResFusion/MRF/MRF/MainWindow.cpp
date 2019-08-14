#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
  
  initToolButtons();

  ui->setupUi(this);    


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
  ui->listWidget->resize(365, 400);
  //设置QListWidget的显示模式
  ui->listWidget->setViewMode(QListView::IconMode);
  //设置QListWidget中单元项的图片大小
  ui->listWidget->setIconSize(QSize(100, 100));
  //设置QListWidget中单元项的间距
  ui->listWidget->setSpacing(10);
  //设置自动适应布局调整（Adjust适应，Fixed不适应），默认不适应
  ui->listWidget->setResizeMode(QListWidget::Adjust);
  //设置不能移动
  ui->listWidget->setMovement(QListWidget::Static);

  //QListWidgetItem *listTitle = new QListWidgetItem(ui->listWidget);
  //listTitle->setIcon(QIcon());
  //listTitle->setText(TR("图片列表"));

  //ui->listWidget->addItem(listTitle);
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

void MainWindow::on_cancleCurrOP_clicked() { 
  enableFileButtons(); 
  m_singleReconstructOp->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(true);
  m_threeFuseThreeOP->setEnabled(true);
}

void MainWindow::on_singleReFile_clicked() { 
  //设置图标disable
  disableFileButtons();
  m_singleResconstructFile->setEnabled(true);
  m_twoFuseThreeOP->setEnabled(false);
  m_threeFuseThreeOP->setEnabled(false);
  //打开图片
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
