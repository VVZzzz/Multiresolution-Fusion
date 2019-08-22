#pragma once
#include <QDialog>
namespace Ui {
class SingleReconDialog;
}

class SingleReconDialog : public QDialog {
  Q_OBJECT
 public:
  explicit SingleReconDialog(QWidget *parent = nullptr,int imgsz = 128);
  const QString getPath() const { return savepath; }
  int getSize() const; 

 private slots:
  void on_toolButton_clicked();
  
 private:
  Ui::SingleReconDialog *ui;
  QString savepath;
  int imgsz;
};
