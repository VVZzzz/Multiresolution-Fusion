#pragma once
#include <QDialog>
#include <QFileInfoList>
namespace Ui {
class TwoFuseDlg;
}

class TwoFuseDlg : public QDialog {
  Q_OBJECT
 public:
   //type 0:已打开单张图 1:已打开序列图
  explicit TwoFuseDlg(QWidget *parent = nullptr,int type = 0);
  const QFileInfoList getInfoList() { return m_fileinfolist; }
  const QString getImgPath() { return m_imgpath; }
  const int getSize() const { return m_imgsz; }
  const QString getSavePath() { return m_savepath; }
  void setType(int type);

 private slots:
  void on_resToolButton_clicked();
  void on_fileToolButton_clicked();

 private:
  Ui::TwoFuseDlg *ui;
  QFileInfoList m_fileinfolist;
  int m_imgsz;
  int m_imgtype;
  QString m_savepath;
  QString m_imgpath;
};
