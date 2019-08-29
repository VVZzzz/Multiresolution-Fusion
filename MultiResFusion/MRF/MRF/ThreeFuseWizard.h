#pragma once
//��ά�ں���ά����
#include <QWizard>
#include <QLabel>
#include <QProgressBar>
#include <QLineEdit>
#include <vector>
#include <QPushButton>
using namespace std;

class IntroPage;
class PorePage;
class SeriesPage;

class FuseWizard : public QWizard {
  Q_OBJECT
 public:
  FuseWizard(QWidget *parent = 0);
  void accept() override;


 public:
  IntroPage *intropage;
  PorePage *porepage;
  SeriesPage *seriespage;
};

//page1: ����ҳ
class IntroPage : public QWizardPage {
  Q_OBJECT
 public:
  IntroPage(QWidget *parent = 0);

 private:
  QLabel *label;
};

//page2: �����׿�϶��ҳ��
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

//page3: ��С������ͼ
class SeriesPage : public QWizardPage {
  Q_OBJECT
 public:
  SeriesPage(QWidget *parent = 0);
  vector<QString> getStrvec() { return strvec; }

 private:
  vector<QString> strvec;
  QLabel *label;
  QLabel *label2;
  QLineEdit *corenum;
  QPushButton *button;
   
};
