#include "childform.h"
#include "ui_childform.h"

ChildForm::ChildForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChildForm)
{
    ui->setupUi(this);
}

ChildForm::~ChildForm()
{
    delete ui;
}



void ChildForm::setTargetInterface(TgtIntf* targetInterface)
{
    ui->_textEdit->setTargetInterface(targetInterface);
}
