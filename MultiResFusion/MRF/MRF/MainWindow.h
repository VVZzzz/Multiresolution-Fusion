#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include <QtWidgets/QMainWindow>
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

 private slots:
  void on_singleReFile_clicked();
  void on_cancleCurrOP_clicked();

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

  //自定义类
  //Thumbnail *m_pThumnail;   //缩略图类(存放读入文件路径)
  std::vector<QString> m_filespath;


};
#endif 
