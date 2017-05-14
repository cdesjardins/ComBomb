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

#include "TgtProcessConnection.h"
#include "unparam.h"
#include <boost/bind/protect.hpp>

std::shared_ptr<TgtProcessIntf> TgtProcessIntf::createProcessConnection(
    const std::shared_ptr<const TgtConnectionConfig>& config)
{
    std::shared_ptr<TgtProcessIntf> ret(new TgtProcessIntf(config));
    // This guy uses tgtMakeConnection otherwise QProcess will result in:
    // QObject: Cannot create children for a parent that is in a different thread
    ret->tgtMakeConnection();
    return ret;
}

void TgtProcessIntf::tgtMakeConnection()
{
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(
        _connectionConfig);
    _proc = new QProcess(this);
    connect(_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readFromStdout()));
    connect(_proc, SIGNAL(readyReadStandardError()), this, SLOT(readFromStderr()));
    connect(_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processDone(int,QProcess::ExitStatus)));
    _proc->setWorkingDirectory(connectionConfig->_workingDir.c_str());
    QStringList args(connectionConfig->_args.c_str());

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TERM", "xterm");
    env.insert("COLORTERM", "gnome-terminal");
    _proc->setProcessEnvironment(env);

    _proc->start(connectionConfig->_program.c_str(), args);
    _processWriterThread = TgtThread::create(boost::protect(std::bind(&TgtProcessIntf::writerThread, this)));
}

TgtProcessIntf::TgtProcessIntf(const std::shared_ptr<const TgtConnectionConfig>& config)
    : TgtIntf(config),
    _proc(nullptr),
    _processMutex(QMutex::Recursive)
{
}

TgtProcessIntf::~TgtProcessIntf()
{
    tgtDisconnect();
    deleteProcess();
}

void TgtProcessIntf::tgtBreakConnection()
{
    _processWriterThread.reset();
}

void TgtProcessIntf::tgtGetTitle(std::string* szTitle)
{
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(
        _connectionConfig);
    *szTitle = connectionConfig->_program;
}

bool TgtProcessIntf::writerThread()
{
    boost::intrusive_ptr<RefCntBuffer> b;

    if (_outgoingData.dequeue(b, 100) == true)
    {
        _processMutex.lock();
        if (_proc != nullptr)
        {
            qint64 sentBytes;
            do
            {
                char* data = boost::asio::buffer_cast<char*>(b->_buffer);
                data[boost::asio::buffer_size(b->_buffer)] = 0;
                sentBytes = _proc->write(data, boost::asio::buffer_size(b->_buffer));
                b->_buffer = boost::asio::buffer(b->_buffer + sentBytes);
            } while ((sentBytes > 0) && (boost::asio::buffer_size(b->_buffer) > 0));
        }
        _processMutex.unlock();
        b.reset();
    }
    return true;
}

void TgtProcessIntf::readFromStdout()
{
    _processMutex.lock();
    QByteArray output;
    if (_proc != nullptr)
    {
        output = _proc->readAllStandardOutput();
    }
    _processMutex.unlock();
    processInput(output);
}

void TgtProcessIntf::readFromStderr()
{
    _processMutex.lock();
    QByteArray output;
    if (_proc != nullptr)
    {
        output = _proc->readAllStandardError();
    }
    _processMutex.unlock();
    processInput(output);
}

void TgtProcessIntf::processInput(const QByteArray& output)
{
    if (output.length() > 0)
    {
        int sentBytes = 0;
        do
        {
            boost::intrusive_ptr<RefCntBuffer> currentIncomingBuffer;
            if (_bufferPool->dequeue(currentIncomingBuffer, 100) == true)
            {
                qint64 len = output.length() - sentBytes;
                size_t copyLen = std::min((size_t)len, boost::asio::buffer_size(currentIncomingBuffer->_buffer) - 1);
                boost::asio::const_buffer b = boost::asio::buffer(output.constData() + sentBytes, len);
                sentBytes += boost::asio::buffer_copy(currentIncomingBuffer->_buffer, b, copyLen);
                currentIncomingBuffer->_buffer = boost::asio::buffer(currentIncomingBuffer->_buffer, copyLen);
                _incomingData.enqueue(currentIncomingBuffer);
            }
            else
            {
                break;
            }
        } while (sentBytes < output.length());
    }
}

void TgtProcessIntf::processError(QProcess::ProcessError error)
{
#ifdef QT_DEBUG
    qDebug("Error %i", error);
#else
    UNREF_PARAM(error);
#endif
}

void TgtProcessIntf::processDone(int, QProcess::ExitStatus)
{
    deleteProcess();
}

void TgtProcessIntf::closeEvent(QCloseEvent*)
{
    deleteProcess();
}

void TgtProcessIntf::deleteProcess()
{
    _processMutex.lock();
    QProcess* p = _proc;
    _proc = nullptr;
    _processMutex.unlock();
    if (p != nullptr)
    {
        delete p;
    }
}
