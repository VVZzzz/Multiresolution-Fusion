#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"


//������������
class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = Q_NULLPTR);
  ~MainWindow();

 private:
  Ui::MainWindowClass *ui;
};
#endif 
