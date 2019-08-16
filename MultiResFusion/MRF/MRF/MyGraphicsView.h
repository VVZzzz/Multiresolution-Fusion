#pragma once
#include <QGraphicsView>
#include <QWheelEvent>
//继承QGraphicsView,重载一些事件
class MyGraphicsView : public QGraphicsView {
  Q_OBJECT
 private:
  int para;

 private slots:
  void onResetView();
 public:
  MyGraphicsView(QWidget *parent = nullptr);

 protected:
  virtual void wheelEvent(QWheelEvent *e) override;
  virtual void mousePressEvent(QMouseEvent *e) override;
};