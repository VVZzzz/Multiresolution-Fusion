#include "MainWindow.h"
#include "tools.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindowClass) {
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

  //在"打开文件"下添加按钮
  //单张三维重建
  QToolButton *singleResconstructFile = new QToolButton(this);
  singleResconstructFile->setText(TR("打开单张三维重建"));
  singleResconstructFile->setToolTip(TR("打开用来三维重建的单张序列图"));
  singleResconstructFile->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  singleResconstructFile->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("单张重建"),
                                 singleResconstructFile);

  //二维融合三维
  QToolButton *twodim1file = new QToolButton(this);
  twodim1file->setText(TR("打开三维重建序列图"));
  twodim1file->setToolTip(TR("打开经过单张三维重建得到的序列图"));
  twodim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  twodim1file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("二维融合三维"),
                                 twodim1file);

  QToolButton *threedim1file = new QToolButton(this);
  threedim1file->setText(TR("打开低分辨大孔序列图"));
  threedim1file->setToolTip(TR("打开低分辨率大孔的序列图"));
  threedim1file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim1file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("二维融合三维"),
                                 threedim1file);

  //三维融合三维
  QToolButton *threedim2file = new QToolButton(this);
  threedim2file->setText(TR("打开高分辨小孔序列图"));
  threedim2file->setToolTip(TR("打开局部小孔高分辨率序列图"));
  threedim2file->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim2file->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 threedim2file);

  QToolButton *threedim_3 = new QToolButton(this);
  threedim_3->setText(TR("打开低分辨大孔序列图"));
  threedim_3->setToolTip(TR("打开低分辨率大孔的序列图"));
  threedim_3->setIcon(QIcon("./Resources/icons/add_folder_2.svg"));
  threedim_3->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("打开文件"), TR("三维融合三维"),
                                 threedim_3);
  

  //在"融合操作"下添加按钮
  QToolButton *singleReconstructOp = new QToolButton(this);
  singleReconstructOp->setText(TR("三维重建"));
  singleReconstructOp->setToolTip(TR("由单张图进行三维重建"));
  singleReconstructOp->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  singleReconstructOp->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 singleReconstructOp);

  QToolButton *twoFuseThreeOP= new QToolButton(this);
  twoFuseThreeOP->setText(TR("二维融合三维"));
  twoFuseThreeOP->setToolTip(TR("将二维重建融合进三维"));
  twoFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  twoFuseThreeOP->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 twoFuseThreeOP);
  QToolButton *threeFuseThreeOP= new QToolButton(this);
  threeFuseThreeOP->setText(TR("三维融合三维"));
  threeFuseThreeOP->setToolTip(TR("将三维融合进三维"));
  threeFuseThreeOP->setIcon(QIcon("./Resources/icons/add_link_2.svg"));
  threeFuseThreeOP->setEnabled(true);
  ui->ribbonTabWidget->addButton(TR("融合操作"), TR("融合操作"),
                                 threeFuseThreeOP);
}

MainWindow::~MainWindow() { delete ui; }
