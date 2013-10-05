#include "cryptlib.h"
#include "TargetIntf.h"
#include <QDebug>
#include <boost/format.hpp>

struct TgtSshImpl
{
    TgtSshImpl(const TgtSshIntf::TgtConnection &config)
        : _tgtConnectionConfig(config),
        _sshThreadRun(true)
    {
    }

    TgtSshIntf::TgtConnection _tgtConnectionConfig;
    CRYPT_SESSION _cryptSession;
    volatile bool _sshThreadRun;
    boost::scoped_ptr<boost::thread> _sshThread;
};

boost::shared_ptr<TgtSshIntf> TgtSshIntf::createSshConnection(const TgtConnection &config)
{
    boost::shared_ptr<TgtSshIntf> ret(new TgtSshIntf(config));
    ret->TgtMakeConnection();
    return ret;
}

TgtSshIntf::TgtSshIntf(const TgtConnection &config)
    : _sshData(new TgtSshImpl(config))
{
}

TgtSshIntf::TgtConnection TgtSshIntf::TgtGetConfig()
{
    return _sshData->_tgtConnectionConfig;
}

void TgtSshIntf::TgtGetErrorMsg(std::string *errmsg, int sts, const std::string &defaultErrMsg)
{
    int status;
    char errorMessage[512];
    int errorMessageLength;
    status = cryptGetAttributeString(_sshData->_cryptSession, CRYPT_ATTRIBUTE_ERRORMESSAGE, errorMessage, &errorMessageLength);
    if (cryptStatusOK(status))
    {
        errorMessage[errorMessageLength] = 0;
        *errmsg = errorMessage;
    }
    else
    {
        boost::format f("%s (%i)");
        *errmsg = str(f % defaultErrMsg % sts);
    }
}

void TgtSshIntf::TgtMakeConnection()
{
    int status;
    std::string errmsg;
    status = cryptCreateSession(&_sshData->_cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to create SSH session");
        throw std::exception(errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_SERVER_NAME,
                                     _sshData->_tgtConnectionConfig._hostName.c_str(),
                                     _sshData->_tgtConnectionConfig._hostName.length());
    if (cryptStatusError(status))
    {
        cryptDestroySession(_sshData->_cryptSession);
        boost::format f("Unable to connect to '%s' (%d)");
        TgtGetErrorMsg(&errmsg, status, str(f % _sshData->_tgtConnectionConfig._hostName % status));
        throw std::exception(errmsg.c_str());
    }
    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_OPTION_NET_CONNECTTIMEOUT, 10);
    if (cryptStatusError(status))
    {
        cryptDestroySession(_sshData->_cryptSession);
        TgtGetErrorMsg(&errmsg, status, "Unable to set ssh connection timeout");
        throw std::exception(errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_USERNAME,
                                     _sshData->_tgtConnectionConfig._userName.c_str(),
                                     _sshData->_tgtConnectionConfig._userName.length());
    if (cryptStatusError(status))
    {
        cryptDestroySession(_sshData->_cryptSession);
        TgtGetErrorMsg(&errmsg, status, "Unable to set username");
        throw std::exception(errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_PASSWORD,
                                     _sshData->_tgtConnectionConfig._password.c_str(),
                                     _sshData->_tgtConnectionConfig._password.length());
    if (cryptStatusError(status))
    {
        cryptDestroySession(_sshData->_cryptSession);
        TgtGetErrorMsg(&errmsg, status, "Unable to set password");
        throw std::exception(errmsg.c_str());
    }
    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_SESSINFO_ACTIVE, true);
    if (cryptStatusError(status))
    {
        cryptDestroySession(_sshData->_cryptSession);
        TgtGetErrorMsg(&errmsg, status, "Unable to activate session");
        throw std::exception(errmsg.c_str());
    }
    else
    {
        _sshData->_sshThread.reset(new boost::thread(boost::bind(&TgtSshIntf::sshThread, this)));
    }
}

TgtSshIntf::~TgtSshIntf ()
{
    TgtDisconnect();
}

int TgtSshIntf::TgtDisconnect()
{
    if (_sshData->_sshThreadRun == true)
    {
        _sshData->_sshThreadRun = false;
        if ((_sshData->_sshThread != NULL) && (_sshData->_sshThread->joinable()))
        {
            _sshData->_sshThread->join();
            _sshData->_sshThread.reset();
        }
    }
    return 0;
}

bool TgtSshIntf::TgtConnected()
{
    return false;
}

void TgtSshIntf::TgtGetTitle(std::string* szTitle)
{
    std::stringstream t;
    t << _sshData->_tgtConnectionConfig._hostName << ":" << _sshData->_tgtConnectionConfig._portNum;
    *szTitle = t.str();
}

void TgtSshIntf::sshThread()
{
    bool attemptReconnect = false;
    while (_sshData->_sshThreadRun == true)
    {
        if ((sshSend() == false) || (sshRecv() == false))
        {
            attemptReconnect = true;
            break;
        }
    }
    cryptDestroySession(_sshData->_cryptSession);
    qDebug("ssh done");
    if (attemptReconnect == true)
    {
        qDebug("attempt reconnect");
        TgtAttemptReconnect();
    }
}

bool TgtSshIntf::sshSend()
{
    int ret = true;
    boost::asio::mutable_buffer b;
    int status;
    int bytesCopied;
    while (_outgoingData.dequeue(b) == true)
    {
        char* data = boost::asio::buffer_cast<char*>(b);
        bytesCopied = 0;
        status = cryptPushData(_sshData->_cryptSession, data, boost::asio::buffer_size(b), &bytesCopied);
        if (cryptStatusError(status))
        {
            qDebug("Unable to send data");
            ret = false;
        }
        else if (bytesCopied < (int)boost::asio::buffer_size(b))
        {
            qDebug("Didnt write everything");
        }
        else
        {
            status = cryptFlushData(_sshData->_cryptSession);
            if (cryptStatusError(status))
            {
                qDebug("Unable to flush data");
                ret = false;
            }
        }
        TgtReturnReadBuffer(b);
    }
    return ret;
}

bool TgtSshIntf::sshRecv()
{
    int outDataLength;
    int status;
    bool ret = true;
    do
    {
        char* data = boost::asio::buffer_cast<char*>(_currentIncomingBuffer);
        outDataLength = 0;
        status = cryptPopData(_sshData->_cryptSession, data, boost::asio::buffer_size(_currentIncomingBuffer), &outDataLength);
        if (cryptStatusError(status))
        {
            ret = false;
        }
        else if (outDataLength > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, outDataLength));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
    } while ((outDataLength > 0) && (cryptStatusOK(status)));
    return ret;
}

