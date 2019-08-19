#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <vector>
#include "ui_MainWindow.h"
#include "pic.h"


//程序主界面类
class MainWindow : public QMainWindow {
  Q_OBJECT

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
  void on_cancleCurrOP_clicked();
  //点击List中项目对应的槽
  void listItem_clicked(QListWidgetItem *item);
  //点击操作按钮
  void on_singleReOP_clicked();
  void on_twoFuseOP_clicked();
  void on_threeFuseOP_clicked();

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
  QToolButton *m_threedim_3;
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

};
#endif 
