#include "singleReconDlg.h"
#include <QFileDialog>
#include <QDir>
#include "tools.h"
#include "ui_singleReconDialog.h"

SingleReconDialog::SingleReconDialog(QWidget *parent /* = nullptr */,int sz)
    : QDialog(parent), ui(new Ui::SingleReconDialog),imgsz(sz) {
  ui->setupUi(this);
  ui->spinBox->setRange(imgsz, 10 * imgsz);
  ui->spinBox->setSingleStep(imgsz);
}

void SingleReconDialog::on_toolButton_clicked() {
  QString dir = QFileDialog::getExistingDirectory(this, TR("±£´æÂ·¾¶"),
                                                  QDir::currentPath());
  if (!dir.isEmpty()) {
    if (ui->dirComboBox->findText(dir) == -1) ui->dirComboBox->addItem(dir);
    ui->dirComboBox->setCurrentIndex(ui->dirComboBox->findText(dir));
  }
  savepath = ui->dirComboBox->currentText();
}

int SingleReconDialog::getSize() const { return ui->spinBox->value(); }
