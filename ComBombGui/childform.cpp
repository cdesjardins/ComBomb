#include "childform.h"
#include "ui_childform.h"

ChildForm::ChildForm(QWidget *parent) :
    QTerminal(parent),
    ui(new Ui::ChildForm)
{
    ui->setupUi(this);
    //setCentralWidget(new QTerminal());
}

ChildForm::~ChildForm()
{
    delete ui;
}

void ChildForm::setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface)
{
    std::string szTitle;

    //ui->_textEdit->setTargetInterface(targetInterface);
    targetInterface->TgtGetTitle(&szTitle);
    setWindowTitle(szTitle.c_str());
}
