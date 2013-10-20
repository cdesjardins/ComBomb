#include "childform.h"
#include "configdialog.h"
#include "ui_childform.h"

ChildForm::ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent) :
    QTerminal(terminalConfig, targetInterface, CB_DEFAULT_TERM_WIDTH, CB_DEFAULT_TERM_HEIGHT, parent),
    ui(new Ui::ChildForm)
{
    std::string szTitle;
    ui->setupUi(this);
    targetInterface->TgtGetTitle(&szTitle);
    setWindowTitle(szTitle.c_str());
    connect(targetInterface.get(), SIGNAL(updateTitleSignal(QString)), this, SLOT(updateTitleSlot(QString)));
    setAttribute(Qt::WA_DeleteOnClose, true);

}

void ChildForm::updateTitleSlot(QString title)
{
    setWindowTitle(title);
}

ChildForm::~ChildForm()
{
    delete ui;
}

