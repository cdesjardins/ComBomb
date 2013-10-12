#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "versioning.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(CB_GIT_VER_STR);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
