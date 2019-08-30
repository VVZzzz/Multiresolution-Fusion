#pragma once
#include <QThread>
#include "Annealing.h"
#include "PoreSet.h"
#include "ThreeFuseWizard.h"
class WorkThread : public QThread {
  Q_OBJECT
 public:
  explicit WorkThread(QObject *parent = 0,CAnnealing *ptr=nullptr,PoreSet *ptr2=nullptr);
  void stop();
  void setAnnealPtr(CAnnealing *ptr);
  void setPoreSetPtr(PoreSet *ptr , Param p);
 signals:
   //若这个工作线程中的操作被手动取消,则发"取消操作"信号
  void opCancle();

 protected:
  void run();

 private:
  volatile bool stopped;
  CAnnealing *pAnnealing;
  PoreSet *pPoreSet;
  Param para;

};
