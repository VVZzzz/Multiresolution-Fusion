#pragma once
//三维融合三维向导类
#include <QWizard>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>
#include <vector>
#include <QPushButton>
#include <QToolButton>
using namespace std;

class IntroPage;
class PorePage;
class SeriesPage;
class SetSizePage;
class FinishPage;

struct Param {
  int smallcorenum;
  int poresetSize;
  double expectPoresity;
  QString savepath;
  vector<QString> smallPathVec;
};

class FuseWizard : public QWizard {
  Q_OBJECT
 public:
  FuseWizard(QWidget *parent = 0);
  void accept() override;
  Param getParam() { return para; }

 signals:
  void SetParaFinish();

 public:
  IntroPage *intropage;
  PorePage *porepage;
  SeriesPage *seriespage;
  SetSizePage *setsizepage;
  FinishPage *finishpage;
  Param para;
};

//page1: 介绍页
class IntroPage : public QWizardPage {
  Q_OBJECT
 public:
  IntroPage(QWidget *parent = 0);

 private:
  QLabel *label;
};

//page2: 计算大孔孔隙度页面
class PorePage : public QWizardPage {
  Q_OBJECT
 public:
  PorePage(QWidget *parent = 0);

 public slots:
  void SetProgress(int val,double pore);

 private:
  QProgressBar *progressbar;
  QLabel *label;
  QLabel *label2;
  QLabel *label3;
  QLineEdit *lineedit;
};

//page3: 打开小孔序列图
class SeriesPage : public QWizardPage {
  Q_OBJECT
 public:
  SeriesPage(QWidget *parent = 0);
  vector<QString> getStrvec() { return strvec; }

 public slots:
  void onBtnClicked();

 private:
  vector<QString> strvec;
  QLabel *label;
  QLabel *label2;
  QLineEdit *corenum;
  QPushButton *button;
   
};

//page4: 设置模式集尺寸与最终保存路径
class SetSizePage : public QWizardPage {
  Q_OBJECT
 public:
  SetSizePage(QWidget *parent = 0);

 public slots:
  void SetSavePath();

 private:
  QLabel *label;
  QLabel *label2;
  QLineEdit *lineedit;
  QLabel *label3;
  QLineEdit *lineedit2;
  QToolButton *btn;
  
};

// page5: 完成页面
class FinishPage : public QWizardPage {
  Q_OBJECT
 public:
  FinishPage(QWidget *parent = 0);

 private:
  QLabel *label;
};
