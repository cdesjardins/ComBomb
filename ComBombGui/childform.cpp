#include "runprocessdialog.h"
#include "mainwindow.h"
#include "childform.h"
#include "configdialog.h"
#include "ui_childform.h"

ChildForm::ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent) :
    QTerminal(terminalConfig, targetInterface, CB_DEFAULT_TERM_WIDTH, CB_DEFAULT_TERM_HEIGHT, parent),
    _mutex(QMutex::Recursive),
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
    connectToRecvText(this);
}

void ChildForm::updateTitleSlot(QString title)
{
    setWindowTitle(title);
}

ChildForm::~ChildForm()
{
    delete ui;
}

void ChildForm::closeEvent(QCloseEvent* event)
{
    deleteProcess();
}

void ChildForm::onReceiveText(const QString& text)
{
    _mutex.lock();
    if (_proc != NULL)
    {
        _proc->write(text.toLocal8Bit().constData(), text.length());
    }
    _mutex.unlock();
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
            _proc->setWorkingDirectory(rpd.getWorkingDirectory());
            QStringList args = rpd.getArguments();
            _proc->start(rpd.getProgram(), args);
        }
    }
    else
    {
        emit updateStatusSignal("Process already running");
    }
}

void ChildForm::readFromStdout()
{
    _mutex.lock();
    if (_proc != NULL)
    {
        sendText(_proc->readAllStandardOutput());
    }
    _mutex.unlock();
}

void ChildForm::processError(QProcess::ProcessError error)
{
    qDebug("Error: %d", error);
    deleteProcess();
}

void ChildForm::processDone(int , QProcess::ExitStatus )
{
    deleteProcess();
}

void ChildForm::deleteProcess()
{
    _mutex.lock();
    QProcess *p = _proc;
    if (_proc != NULL)
    {
        _proc = NULL;
        delete p;
    }
    _mutex.unlock();
}
