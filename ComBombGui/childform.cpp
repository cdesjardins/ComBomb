/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
        qint64 sentBytes;
        do
        {
            char* buf = boost::asio::buffer_cast<char*>(incoming->_buffer);
            sentBytes = _proc->write(buf, boost::asio::buffer_size(incoming->_buffer));
            incoming->_buffer = boost::asio::buffer(incoming->_buffer + sentBytes);
        } while ((sentBytes > 0) && (boost::asio::buffer_size(incoming->_buffer) > 0));
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
    // If the process failed to start, then it
    // cannot be deleted from the processError call
    // because it may be called from within the call to
    // _proc->start()... so _proc cannot be deleted...
    if ((_proc != NULL) && (_procError == true))
    {
        deleteProcess();
    }
    if (_proc == NULL)
    {
        if (rpd.exec() == RunProcessDialog::Accepted)
        {
            _procError = false;
            _redirectStdout = rpd.isStdoutRedirected();
            _redirectStderr = rpd.isStderrRedirected();

            _proc = new QProcess(this);
            connect(_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromStdout()));
            connect(_proc, SIGNAL(readyReadStandardError()), this, SLOT(readFromStderr()));
            connect(_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
            connect(_proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processDone(int, QProcess::ExitStatus)));
            _proc->setWorkingDirectory(rpd.getWorkingDirectory());
            QStringList args = rpd.getArguments();
            suppressOutput(rpd.isOutputSuppressed());
            MainWindow::getMainWindow()->swapProcessIcon(true);
            _proc->start(rpd.getProgram(), args);
        }
    }
    else
    {
        deleteProcess();
        emit updateStatusSignal("Stopping process");
    }
}

void ChildForm::readFromStdout()
{
    readFromProc(true);
}

void ChildForm::readFromStderr()
{
    readFromProc(false);
}

void ChildForm::readFromProc(bool isStdout)
{
    bool redirect = false;
    _processMutex.lock();
    QByteArray output;
    if (_proc != NULL)
    {
        if (isStdout == true)
        {
            output = _proc->readAllStandardOutput();
            redirect = _redirectStdout;
        }
        else
        {
            output = _proc->readAllStandardError();
            redirect = _redirectStderr;
        }
    }
    _processMutex.unlock();
    if (output.length() > 0)
    {
        if (redirect == true)
        {
            sendText(output);
        }
        else
        {
            recvText(output);
        }
    }
}

void ChildForm::processError(QProcess::ProcessError error)
{
    QString errMsg;
    QString errors[] =
    {
        "failed to start",
        "crashed",
        "timedout",
        "read error",
        "write error",
        "unknown error"
    };
    _procError = true;
    suppressOutput(false);
    errMsg.sprintf("Error: (%d) Process ", error);
    if (error < sizeof(errors) / sizeof(errors[0]))
    {
        errMsg.append(errors[error].toLocal8Bit().constData());
    }
    else
    {
        errMsg.append("error");
    }
    emit updateStatusSignal(errMsg);
    MainWindow::getMainWindow()->swapProcessIcon(false);
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
    MainWindow::getMainWindow()->swapProcessIcon(false);
}
