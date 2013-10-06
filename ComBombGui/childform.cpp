#include "childform.h"
#include "ui_childform.h"

ChildForm::ChildForm(const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent) :
    QTerminal(targetInterface, CB_DEFAULT_TERM_WIDTH, CB_DEFAULT_TERM_HEIGHT, parent),
    ui(new Ui::ChildForm)
{
    std::string szTitle;
    ui->setupUi(this);
    targetInterface->TgtGetTitle(&szTitle);
    setWindowTitle(szTitle.c_str());
    setAttribute(Qt::WA_DeleteOnClose, true);
}

ChildForm::~ChildForm()
{
    delete ui;
}

