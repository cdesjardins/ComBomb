#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "versioning.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    QString ver("ComBomb version: ");
    ver.append(CB_GIT_VER_STR);
    ui->versionLabel->setText(ver);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
