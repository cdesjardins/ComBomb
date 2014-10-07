#include "runprocessdialog.h"
#include "finddialog.h"
#include "mainwindow.h"
#include "childform.h"
#include "configdialog.h"
#include "ui_childform.h"

ChildForm::ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent) :
    QTerminal(terminalConfig, targetInterface, CB_DEFAULT_TERM_WIDTH, CB_DEFAULT_TERM_HEIGHT, parent),
    _processMutex(QMutex::Recursive),
    ui(new Ui::ChildForm),
    _proc(NULL),
    _procError(false)
{
    std::string szTitle;
    ui->setupUi(this);

    targetInterface->tgtGetTitle(&szTitle);
    setWindowTitle(szTitle.c_str());
    connect(targetInterface.get(), SIGNAL(updateTitleSignal(QString)), this, SLOT(updateTitleSlot(QString)));
    connect(this, SIGNAL(updateStatusSignal(QString)), MainWindow::getMainWindow(), SLOT(updateStatusSlot(QString)));
    connect(this, SIGNAL(openWindowSignal()), MainWindow::getMainWindow(), SLOT(openWindowSlot()));
    connect(this, SIGNAL(closeWindowSignal()), MainWindow::getMainWindow(), SLOT(closeWindowSlot()));
    setAttribute(Qt::WA_DeleteOnClose, true);
    connectToRecvText(this);
    emit openWindowSignal();
}

void ChildForm::updateTitleSlot(QString title)
{
    setWindowTitle(title);
}

ChildForm::~ChildForm()
{
    delete ui;
}

void ChildForm::closeEvent(QCloseEvent* )
{
    deleteProcess();
    emit closeWindowSignal();
}

void ChildForm::onReceiveBlock(boost::intrusive_ptr<RefCntBuffer> incoming)
{
    _processMutex.lock();
    if (_proc != NULL)
    {
        char* buf = boost::asio::buffer_cast<char*>(incoming->_buffer);
        int len = boost::asio::buffer_size(incoming->_buffer);
        _proc->write(buf, len);
    }
    _processMutex.unlock();
}

void ChildForm::findText()
{
    FindDialog fd(this);
    if (fd.exec() == FindDialog::Accepted)
    {
        _lastSearchString = fd.getString();
        _lastSearchCaseSensitivity = fd.getCaseSensitivity();
        QTerminal::findText(fd.getString(), fd.getCaseSensitivity(), fd.getSearchUp(), false);
    }
}

void ChildForm::findTextNext(const bool backward)
{
    if (_lastSearchString.isNull() == false)
    {
        QTerminal::findText(_lastSearchString, _lastSearchCaseSensitivity, backward, true);
    }
    else
    {
        {
            // new scope to force destructor
            FindDialog fd(this);
            fd.setSearchUp(backward);
        }
        findText();
    }
}

void ChildForm::findTextHighlighted()
{
    FindDialog fd(this);
    QString lastSearchStr = QTerminal::findTextHighlighted(fd.getCaseSensitivity());
    if (lastSearchStr.length() > 0)
    {
        _lastSearchString = lastSearchStr;
        fd.addString(_lastSearchString);
    }
    else
    {
        emit updateStatusSignal("No text is highlighted");
    }
}

void ChildForm::runProcess()
{
    RunProcessDialog rpd(this);
    if ((_proc != NULL) && (_procError == true))
    {
        deleteProcess();
    }
    if (_proc == NULL)
    {
        if (rpd.exec() == RunProcessDialog::Accepted)
        {
            _procError = false;
            _proc = new QProcess(this);
            connect(_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromStdout()));
            connect(_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
            connect(_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processDone(int, QProcess::ExitStatus)));
            _proc->setWorkingDirectory(rpd.getWorkingDirectory());
            QStringList args = rpd.getArguments();
            suppressOutput(rpd.isOutputSuppressed());
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
    _processMutex.lock();
    if (_proc != NULL)
    {
        sendText(_proc->readAllStandardOutput());
    }
    _processMutex.unlock();
}

void ChildForm::processError(QProcess::ProcessError error)
{
    QString errors[] =
    {
        "Failed To Start",
        "Crashed",
        "Timedout",
        "Read Error",
        "Write Error",
        "Unknown Error"
    };
    _procError = true;
    suppressOutput(false);
    QString errMsg;
    errMsg.sprintf("Error: (%d) %s", error, errors[error].toLocal8Bit().constData());
    emit updateStatusSignal(errMsg);
}

void ChildForm::processDone(int , QProcess::ExitStatus )
{
    deleteProcess();
}

void ChildForm::deleteProcess()
{
    _processMutex.lock();
    QProcess *p = _proc;
    _proc = NULL;
    _processMutex.unlock();
    if (p != NULL)
    {
        delete p;
    }
    suppressOutput(false);
}
