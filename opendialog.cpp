#include "opendialog.h"
#include "ui_opendialog.h"

OpenDialog::OpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenDialog)
{
    ui->setupUi(this);
}

OpenDialog::~OpenDialog()
{
    delete ui;
}
