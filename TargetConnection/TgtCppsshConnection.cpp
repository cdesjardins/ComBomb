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
#include "cppssh/cppssh.h"
#include "TgtCppsshConnection.h"
#include "TgtThread.h"
#include "CBException.h"
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <sstream>

class TgtCppsshInit
{
public:
    static std::shared_ptr<TgtCppsshInit> instance()
    {
        std::shared_ptr<TgtCppsshInit> tmp = _instance;
        if (!tmp)
        {
            std::unique_lock<std::mutex> guard(_instantiationMutex);
            tmp = _instance;
            if (!tmp)
            {
                _instance.reset(new TgtCppsshInit);
                tmp = _instance;
            }
        }
        return tmp;
    }

    TgtCppsshInit()
    {
        Cppssh::create();
    }

    ~TgtCppsshInit()
    {
        Cppssh::destroy();
    }

private:
    static std::shared_ptr<TgtCppsshInit> _instance;
    static std::mutex _instantiationMutex;
};

std::shared_ptr<TgtCppsshInit> TgtCppsshInit::_instance;
std::mutex TgtCppsshInit::_instantiationMutex;

struct TgtCppsshImpl
{
    TgtCppsshImpl()
        : _sshInit(TgtCppsshInit::instance()),
          _connectionId(-1)
    {
    }

    ~TgtCppsshImpl()
    {
        _sshInit.reset();
    }

    std::shared_ptr<TgtThread> _sshThread;
    std::shared_ptr<TgtCppsshInit> _sshInit;
    int _connectionId;
};

std::shared_ptr<TgtCppsshIntf> TgtCppsshIntf::createCppsshConnection(const std::shared_ptr<const TgtConnectionConfig>& config)
{
    std::shared_ptr<TgtCppsshIntf> ret(new TgtCppsshIntf(config));
    ret->tgtAttemptReconnect();
    return ret;
}

TgtCppsshIntf::TgtCppsshIntf(const std::shared_ptr<const TgtConnectionConfig>& config)
    : TgtIntf(config),
    _sshData(new TgtCppsshImpl())
{
}

TgtCppsshIntf::~TgtCppsshIntf()
{
    tgtDisconnect();
    _sshData.reset();
}

void TgtCppsshIntf::tgtGetErrorMsg(std::string* errmsg, int sts, const std::string& defaultErrMsg)
{
}

void TgtCppsshIntf::tgtMakeConnection()
{
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);

    if (Cppssh::connect(&_sshData->_connectionId, connectionConfig->_hostName.c_str(),
                        connectionConfig->_portNum, connectionConfig->_userName.c_str(),
                        connectionConfig->_password.c_str()) == true)
    {
        _sshData->_sshThread = TgtThread::create(boost::bind(std::bind(&TgtCppsshIntf::sshThread, this)));
    }
}

bool TgtCppsshIntf::tryPrivateKey(std::shared_ptr<const TgtConnectionConfig> connectionConfig)
{
    bool ret = false;

    return ret;
}

void TgtCppsshIntf::tgtBreakConnection()
{
    _sshData->_sshThread.reset();
}

void TgtCppsshIntf::tgtGetTitle(std::string* szTitle)
{
    std::shared_ptr<const TgtConnectionConfig> connectionConfig = std::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    std::stringstream t;
    t << connectionConfig->_hostName << ":" << connectionConfig->_portNum;
    *szTitle = t.str();
}

bool TgtCppsshIntf::sshThread()
{
    bool attemptReconnect = false;
    if ((Cppssh::isConnected(_sshData->_connectionId) == false) || (sshSend() == false) || (sshRecv() == false))
    {
        attemptReconnect = true;
    }

    if (attemptReconnect == true)
    {
        tgtAttemptReconnect();
    }
    return !attemptReconnect;
}

bool TgtCppsshIntf::sshSend()
{
    int ret = true;
    boost::intrusive_ptr<RefCntBuffer> b;

    while (_outgoingData.dequeue(b) == true)
    {
        uint8_t* data = boost::asio::buffer_cast<uint8_t*>(b->_buffer);

        if (Cppssh::write(_sshData->_connectionId, data, boost::asio::buffer_size(b->_buffer)) == false)
        {
            // FIXME: Report error
        }
    }
    return ret;
}

bool TgtCppsshIntf::sshRecv()
{
    bool ret = true;

    size_t sentBytes;
    do
    {
        CppsshMessage msg;

        sentBytes = 0;
        if ((Cppssh::read(_sshData->_connectionId, &msg) == true) && (msg.length() > 0))
        {
            size_t length = msg.length();
            while ((sentBytes < length) && (ret == true) && (_sshData->_sshThread->threadRun() == true))
            {
                boost::intrusive_ptr<RefCntBuffer> currentIncomingBuffer;
                if (_bufferPool->dequeue(currentIncomingBuffer, 100) == true)
                {
                    qint64 len = msg.length() - sentBytes;
                    size_t copyLen = std::min((size_t)len, boost::asio::buffer_size(currentIncomingBuffer->_buffer) - 1);
                    boost::asio::const_buffer b = boost::asio::buffer(msg.message() + sentBytes, len);
                    sentBytes += boost::asio::buffer_copy(currentIncomingBuffer->_buffer, b, copyLen);
                    currentIncomingBuffer->_buffer = boost::asio::buffer(currentIncomingBuffer->_buffer, copyLen);
                    _incomingData.enqueue(currentIncomingBuffer);
                }
                else
                {
                    ret = false;
                    break;
                }
            }
        }
    } while ((sentBytes > 0) && (_sshData->_sshThread->threadRun() == true));
    return ret;
}

