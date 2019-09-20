#include "WorkThread.h"
#include <QDebug>
WorkThread::WorkThread(QObject *parent /* = 0 */,
                       CAnnealing *ptr /* = nullptr */,
                       PoreSet *ptr2 /* = nullptr*/)
    : QThread(parent) {
  stopped = false;
  pAnnealing = nullptr;
  pPoreSet = nullptr;
}

void WorkThread::stop() { qDebug() << "cancle op"; }

void WorkThread::setAnnealPtr(CAnnealing *ptr) { 
  pAnnealing = ptr; }

void WorkThread::setPoreSetPtr(PoreSet *ptr, Param p) {
  pPoreSet = ptr;
  para = p;
}

void WorkThread::run() { 
  if (pAnnealing) {
    if (!pAnnealing->Work()) {
      emit opCancle();
    } 
  } else if(pPoreSet){
    pPoreSet->SetNumSize(para.smallcorenum, para.poresetSize);
    pPoreSet->SetExpectPorosity(para.expectPoresity);
    if (!pPoreSet->LoadSmallPoreSet(para.smallPathVec)) {
      emit opCancle();
      return;
    }
    if (!pPoreSet->Reconstruct(para.savepath)) {
      emit opCancle();
      return;
    }
  }
}

