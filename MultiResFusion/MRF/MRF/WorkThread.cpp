#include "WorkThread.h"
#include <QDebug>
WorkThread::WorkThread(QObject *parent /* = 0 */,
                       CAnnealing *ptr /* = nullptr */)
    : QThread(parent) {
  stopped = false;
  pAnnealing = nullptr;
}

void WorkThread::stop() { qDebug() << "cancle op"; }

void WorkThread::setAnnealPtr(CAnnealing *ptr) { 
  pAnnealing = ptr;
}

void WorkThread::run() { 
  if (pAnnealing) {
    if (!pAnnealing->Reconstruct()) {
      emit opCancle();
    } 
  }
}

