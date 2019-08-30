#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <vector>
#include "ui_MainWindow.h"
#include "pic.h"
#include "singleReconDlg.h"
#include "Annealing.h"
#include "WorkThread.h"
#include "TwoFuseDlg.h"
#include "PoreSet.h"
#include "ThreeFuseWizard.h"


//程序主界面类
class MainWindow : public QMainWindow {
  Q_OBJECT
  enum OPType {
    SINGLE,       //单张二维图
    LOW2SERIES,   //低分辨率大孔序列图(2融合3用)
    HIGH3SERIES,  //高分辨率小孔序列图(3融合3用)
    LOW3SERIES,   //低分辨率大孔序列图(3融合3用)
    EMPTY         //默认值(空)
  };

 public:
  MainWindow(QWidget *parent = Q_NULLPTR);
  ~MainWindow();

  private:
  void initToolButtons();
  void initListWidget();
  void disableFileButtons();
  void enableFileButtons();
  void clearView();
  void clearListWidget();
  void addImage2View(const QString &);
  void addImage2List(const QString &);

  bool openSingleImg();
  bool openSeriesImg();

 private slots:
  void on_singleReFile_clicked();
  void on_twodim1_clicked();
  void on_threedim1_clicked();
  void on_threedim2_clicked();
  void on_threedim3_clicked();

  void on_cancleCurrOP_clicked();
  //点击List中项目对应的槽
  void listItem_clicked(QListWidgetItem *item);
  //点击操作按钮
  void on_singleReOP_clicked();
  void on_twoFuseOP_clicked();
  void on_threeFuseOP_clicked();
  //工作线程完成
  void on_workthread_finished();
  //收到工作线程进度
  void on_progress(int);
  //工作线程被取消操作
  void on_canlethread();
  //三维融合三维参数设置完成,开始进行重建操作
  void on_startThreeFuse();

 private:
  Ui::MainWindowClass *ui;

  //界面上自定义部件

  //"打开文件"中按钮
  //单张三维重建
  QToolButton *m_singleResconstructFile;
  //二维融合三维
  QToolButton *m_twodim1file;
  QToolButton *m_threedim1file;
  //三维融合三维
  QToolButton *m_threedim2file;
  QToolButton *m_threedim3file;
  //"融合操作"中按钮
  QToolButton *m_singleReconstructOp;
  QToolButton *m_twoFuseThreeOP;
  QToolButton *m_threeFuseThreeOP;
  //"工具"中按钮
  QToolButton *m_cancleCurrOP;

  //状态栏中的label
  QLabel *m_sizeLabel;


  //场景
  QGraphicsScene *m_graphScene;

  //数据类
  std::vector<QString> m_filespath;  //存放读入图片的路径
  QPixmap m_pixmap;                  //图片缩略图
  QFileInfoList m_fileinfolist;

  //自定义对话框类
  SingleReconDialog *m_singleReconDlg;  //三维重建对话框
  TwoFuseDlg *m_twoFuseDlg;

  //三维融合向导页类
  FuseWizard *m_pFuseWizard;

  //三维重建,二维融合三维核心类
  CAnnealing *m_pCAnneal;

  //三维融合三维核心类
  PoreSet *m_pPoreset;

  //工作线程
  WorkThread *m_pWorkthread;

  //读入图的类型
  OPType m_imgtype;

  
  
  

};
#endif 
