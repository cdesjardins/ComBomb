#include "runprocessdialog.h"
#include "mainwindow.h"
#include "childform.h"
#include "configdialog.h"
#include "ui_childform.h"

ChildForm::ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent) :
    QTerminal(terminalConfig, targetInterface, CB_DEFAULT_TERM_WIDTH, CB_DEFAULT_TERM_HEIGHT, parent),
    ui(new Ui::ChildForm),
    _proc(NULL)
{
    std::string szTitle;
    ui->setupUi(this);
    targetInterface->TgtGetTitle(&szTitle);
    setWindowTitle(szTitle.c_str());
    connect(targetInterface.get(), SIGNAL(updateTitleSignal(QString)), this, SLOT(updateTitleSlot(QString)));
    connect(this, SIGNAL(updateStatusSignal(QString)), MainWindow::getMainWindow(), SLOT(updateStatusSlot(QString)));
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

void ChildForm::runProcess()
{
    RunProcessDialog rpd;
    if (_proc == NULL)
    {
        if (rpd.exec() == RunProcessDialog::Accepted)
        {
            _proc = new QProcess(this);
            connect(_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromStdout()));
            connect(_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
            connect(_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processDone(int, QProcess::ExitStatus)));
            //_proc->start("C:\\Users\\ChrisD\\software_devel\\tmp\\Debug\\tmp.exe");
        }
    }
    else
    {
        emit updateStatusSignal("Process already running");
    }
}

void ChildForm::readFromStdout()
{
    sendText(_proc->readAllStandardOutput());
}

void ChildForm::processError(QProcess::ProcessError error)
{
    qDebug("Error: %d", error);
    delete _proc;
    _proc = NULL;
}

void ChildForm::processDone(int , QProcess::ExitStatus )
{
    delete _proc;
    _proc = NULL;
}
