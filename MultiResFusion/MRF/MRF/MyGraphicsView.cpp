#include "MyGraphicsView.h"
#include "tools.h"
#include <QMenu>
#include <QDebug>

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : QGraphicsView(parent), para(0) {
  //�����Ҽ��˵�����
  this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MyGraphicsView::wheelEvent(QWheelEvent* e) {
  QPoint numDegrees = e->angleDelta() / 8;
  QPoint numSteps = numDegrees / 15;
  qreal val = pow(1.2, numSteps.y());
  this->scale(val, val);  //�Ŵ���С
}

void MyGraphicsView::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::RightButton) {
   //�����˵�����
    QMenu *pMenu = new QMenu(this);

    QAction *pReset = new QAction(TR("������ͼ"),pMenu);

    //1:�½����� 2:�������� 3:ɾ������ 4:�������� 5:���ù��� 6:ɾ������
    //1:������ͼ
    pReset->setData(1);
    /*
    pEditTask->setData(2);
    pDeleteTask ->setData(3);
    pToolRenName->setData(4);
    pToolEdot->setData(5);
    pToolDelete ->setData(6);
    */

    //��QAction������ӵ��˵���
    pMenu->addAction(pReset);

    //��������Ҽ�����ź�
    connect(pReset, SIGNAL(triggered()), this, SLOT(onResetView()));

    //������Ҽ�����ĵط���ʾ�˵�
    pMenu->exec(cursor().pos());

    //�ͷ�����˵�,����Action�������Ӳ������Զ�ɾ��
    delete pMenu;
  }
}


void MyGraphicsView::onResetView() { 
  //qDebug() << "resetView";
  this->resetTransform();
}
