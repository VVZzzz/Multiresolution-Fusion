#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include <QtWidgets/QMainWindow>
#include <vector>
#include "ui_MainWindow.h"
#include "pic.h"


//������������
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

  //�������Զ��岿��

  //"���ļ�"�а�ť
  //������ά�ؽ�
  QToolButton *m_singleResconstructFile;
  //��ά�ں���ά
  QToolButton *m_twodim1file;
  QToolButton *m_threedim1file;
  //��ά�ں���ά
  QToolButton *m_threedim2file;
  QToolButton *m_threedim_3;

  //"�ںϲ���"�а�ť
  QToolButton *m_singleReconstructOp;
  QToolButton *m_twoFuseThreeOP;
  QToolButton *m_threeFuseThreeOP;

  //"����"�а�ť
  QToolButton *m_cancleCurrOP;

  //�Զ�����
  //Thumbnail *m_pThumnail;   //����ͼ��(��Ŷ����ļ�·��)
  std::vector<QString> m_filespath;


};
#endif 
