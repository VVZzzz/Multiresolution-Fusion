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


//������������
class MainWindow : public QMainWindow {
  Q_OBJECT
  enum OPType {
    SINGLE,       //���Ŷ�άͼ
    LOW2SERIES,   //�ͷֱ��ʴ������ͼ(2�ں�3��)
    HIGH3SERIES,  //�߷ֱ���С������ͼ(3�ں�3��)
    LOW3SERIES,   //�ͷֱ��ʴ������ͼ(3�ں�3��)
    EMPTY         //Ĭ��ֵ(��)
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
  //���List����Ŀ��Ӧ�Ĳ�
  void listItem_clicked(QListWidgetItem *item);
  //���������ť
  void on_singleReOP_clicked();
  void on_twoFuseOP_clicked();
  void on_threeFuseOP_clicked();
  //�����߳����
  void on_workthread_finished();
  //�յ������߳̽���
  void on_progress(int);
  //�����̱߳�ȡ������
  void on_canlethread();
  //��ά�ں���ά�����������,��ʼ�����ؽ�����
  void on_startThreeFuse();

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
  QToolButton *m_threedim3file;
  //"�ںϲ���"�а�ť
  QToolButton *m_singleReconstructOp;
  QToolButton *m_twoFuseThreeOP;
  QToolButton *m_threeFuseThreeOP;
  //"����"�а�ť
  QToolButton *m_cancleCurrOP;

  //״̬���е�label
  QLabel *m_sizeLabel;


  //����
  QGraphicsScene *m_graphScene;

  //������
  std::vector<QString> m_filespath;  //��Ŷ���ͼƬ��·��
  QPixmap m_pixmap;                  //ͼƬ����ͼ
  QFileInfoList m_fileinfolist;

  //�Զ���Ի�����
  SingleReconDialog *m_singleReconDlg;  //��ά�ؽ��Ի���
  TwoFuseDlg *m_twoFuseDlg;

  //��ά�ں���ҳ��
  FuseWizard *m_pFuseWizard;

  //��ά�ؽ�,��ά�ں���ά������
  CAnnealing *m_pCAnneal;

  //��ά�ں���ά������
  PoreSet *m_pPoreset;

  //�����߳�
  WorkThread *m_pWorkthread;

  //����ͼ������
  OPType m_imgtype;

  
  
  

};
#endif 
