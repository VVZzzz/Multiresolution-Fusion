#pragma once
#include <QThread>
#include "Annealing.h"
class WorkThread : public QThread {
  Q_OBJECT
 public:
  explicit WorkThread(QObject *parent = 0,CAnnealing *ptr=nullptr);
  void stop();
  void setAnnealPtr(CAnnealing *ptr);
 signals:
   //����������߳��еĲ������ֶ�ȡ��,��"ȡ������"�ź�
  void opCancle();

 protected:
  void run();

 private:
  volatile bool stopped;
  CAnnealing *pAnnealing;

};
