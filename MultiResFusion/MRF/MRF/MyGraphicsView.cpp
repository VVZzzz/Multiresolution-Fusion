#include "MyGraphicsView.h"
#include "tools.h"
#include <QMenu>
#include <QDebug>

MyGraphicsView::MyGraphicsView(QWidget* parent)
    : QGraphicsView(parent), para(0) {
  //设置右键菜单属性
  this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MyGraphicsView::wheelEvent(QWheelEvent* e) {
  QPoint numDegrees = e->angleDelta() / 8;
  QPoint numSteps = numDegrees / 15;
  qreal val = pow(1.2, numSteps.y());
  this->scale(val, val);  //放大缩小
}

void MyGraphicsView::mousePressEvent(QMouseEvent *e) {
  if (e->button() == Qt::RightButton) {
   //创建菜单对象
    QMenu *pMenu = new QMenu(this);

    QAction *pReset = new QAction(TR("重置视图"),pMenu);

    //1:新建任务 2:设置任务 3:删除任务 4:改名工具 5:设置工具 6:删除工具
    //1:重置视图
    pReset->setData(1);
    /*
    pEditTask->setData(2);
    pDeleteTask ->setData(3);
    pToolRenName->setData(4);
    pToolEdot->setData(5);
    pToolDelete ->setData(6);
    */

    //把QAction对象添加到菜单上
    pMenu->addAction(pReset);

    //连接鼠标右键点击信号
    connect(pReset, SIGNAL(triggered()), this, SLOT(onResetView()));

    //在鼠标右键点击的地方显示菜单
    pMenu->exec(cursor().pos());

    //释放这个菜单,其中Action是它的子部件会自动删除
    delete pMenu;
  }
}


void MyGraphicsView::onResetView() { 
  //qDebug() << "resetView";
  this->resetTransform();
}
