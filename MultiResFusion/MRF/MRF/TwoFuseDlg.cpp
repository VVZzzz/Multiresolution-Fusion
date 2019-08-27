#include "ui_TwoFuseDialog.h"
#include "TwoFuseDlg.h"
#include "tools.h"
#include <QFileDialog>
#include <QImage>

TwoFuseDlg::TwoFuseDlg(QWidget *parent /* = nullptr */, int type /* = 0 */)
    : QDialog(parent), m_imgtype(type), ui(new Ui::TwoFuseDlg) {
  
  ui->setupUi(this);
}

void TwoFuseDlg::on_resToolButton_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, TR("保存路径"),
                                                  QDir::currentPath());
  if (!dir.isEmpty()) {
    if (ui->resDirComboBox->findText(dir) == -1) ui->resDirComboBox->addItem(dir);
    ui->resDirComboBox->setCurrentIndex(ui->resDirComboBox->findText(dir));
  }
  m_savepath = ui->resDirComboBox->currentText();
}


void TwoFuseDlg::on_fileToolButton_clicked() {
  m_imgpath = QFileDialog::getOpenFileName(
      this, tr("Open Image"), ".",
      tr("Image Files (*.png *.jpg *.bmp *.jpeg)"));
  if (!m_imgtype) {
    QDir dir(QFileInfo(m_imgpath).absoluteDir());
    QStringList filters;
    filters << "*.png"
            << "*.jpg"
            << "*.bmp"
            << "*.jpeg";
    dir.setNameFilters(filters);
    m_fileinfolist = dir.entryInfoList();
  }

  if (!m_imgpath.isEmpty()) {
    if (ui->fileDirComboBox->findText(m_imgpath) == -1) ui->fileDirComboBox->addItem(m_imgpath);
    ui->fileDirComboBox->setCurrentIndex(ui->fileDirComboBox->findText(m_imgpath));
  }
}

void TwoFuseDlg::setType(int type) {
  m_imgtype = type;
  if (!type) 
    ui->fileLabel->setText(TR("低分辨率大孔路径:"));
  else 
    ui->fileLabel->setText(TR("单张图片路径:"));
  
}
