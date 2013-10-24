#include "cl/cryptlib.h"
#include "TgtSshConnection.h"
#include "CBException.h"
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class TgtSshInit
{
public:
    static boost::shared_ptr<TgtSshInit> instance()
    {
        boost::shared_ptr<TgtSshInit> tmp = _instance;
        if (!tmp)
        {
            boost::mutex::scoped_lock guard(_instantiationMutex);
            tmp = _instance;
            if (!tmp)
            {
                _instance.reset(new TgtSshInit);
                tmp = _instance;
            }
        }
        return tmp;
    }

    TgtSshInit()
    {
        C_RET status = cryptInit();
        if (cryptStatusError(status))
        {
            throw CB_EXCEPTION_STR(CBException::CbExcp, "Unable to init cryptLib");
        }
    }

    ~TgtSshInit()
    {
        cryptEnd();
    }

private:
    static boost::shared_ptr<TgtSshInit> _instance;
    static boost::mutex _instantiationMutex;
};

boost::shared_ptr<TgtSshInit> TgtSshInit::_instance;
boost::mutex TgtSshInit::_instantiationMutex;

struct TgtSshImpl
{
    TgtSshImpl()
        : _sshThreadRun(true),
        _sshInit(TgtSshInit::instance())
    {
    }

    ~TgtSshImpl()
    {
        _sshInit.reset();
    }

    CRYPT_SESSION _cryptSession;
    volatile bool _sshThreadRun;
    boost::scoped_ptr<boost::thread> _sshThread;
    boost::shared_ptr<TgtSshInit> _sshInit;
};

boost::shared_ptr<TgtSshIntf> TgtSshIntf::createSshConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtSshIntf> ret(new TgtSshIntf(config));
    ret->TgtMakeConnection();
    return ret;
}

TgtSshIntf::TgtSshIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config),
    _sshData(new TgtSshImpl())
{
}

TgtSshIntf::~TgtSshIntf()
{
    tgtDisconnect();
    _sshData.reset();
}

void TgtSshIntf::TgtGetErrorMsg(std::string* errmsg, int sts, const std::string &defaultErrMsg)
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
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);

    status = cryptCreateSession(&_sshData->_cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to create SSH session");
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_SERVER_NAME,
                                     connectionConfig->_hostName.c_str(),
                                     connectionConfig->_hostName.length());
    if (cryptStatusError(status))
    {
        boost::format f("Unable to connect to '%s' (%d)");
        TgtGetErrorMsg(&errmsg, status, str(f % connectionConfig->_hostName % status));
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_OPTION_NET_CONNECTTIMEOUT, 10);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set ssh connection timeout");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    std::string tt("xterm-color");
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_TERM_TYPE,
                                     tt.c_str(), tt.length());
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set terminal type");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession,
                               CRYPT_SESSINFO_TERM_WIDTH, CB_DEFAULT_TERM_WIDTH);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set terminal width");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession,
                               CRYPT_SESSINFO_TERM_HEIGHT, CB_DEFAULT_TERM_HEIGHT);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set terminal height");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_USERNAME,
                                     connectionConfig->_userName.c_str(),
                                     connectionConfig->_userName.length());
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set username");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_PASSWORD,
                                     connectionConfig->_password.c_str(),
                                     connectionConfig->_password.length());
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to set password");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_SESSINFO_ACTIVE, true);
    if (cryptStatusError(status))
    {
        TgtGetErrorMsg(&errmsg, status, "Unable to activate session");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    else
    {
        _sshData->_sshThread.reset(new boost::thread(boost::bind(&TgtSshIntf::sshThread, this)));
    }
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
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    std::stringstream t;
    t << connectionConfig->_hostName << ":" << connectionConfig->_portNum;
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
    if (attemptReconnect == true)
    {
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

