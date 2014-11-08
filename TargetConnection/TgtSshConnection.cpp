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
#include "cl/cryptlib.h"
#include "TgtSshConnection.h"
#include "CBException.h"
#include <boost/format.hpp>
#include <boost/thread.hpp>
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
    boost::intrusive_ptr<RefCntBuffer> _currentIncomingBuffer;
};

boost::shared_ptr<TgtSshIntf> TgtSshIntf::createSshConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtSshIntf> ret(new TgtSshIntf(config));
    ret->tgtMakeConnection();
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

void TgtSshIntf::tgtGetErrorMsg(std::string* errmsg, int sts, const std::string &defaultErrMsg)
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

void TgtSshIntf::tgtMakeConnection()
{
    int status;
    std::string errmsg;
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);

    status = cryptCreateSession(&_sshData->_cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to create SSH session");
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_SERVER_NAME,
                                     connectionConfig->_hostName.c_str(),
                                     connectionConfig->_hostName.length());
    if (cryptStatusError(status))
    {
        boost::format f("Unable to connect to '%s' (%d)");
        tgtGetErrorMsg(&errmsg, status, str(f % connectionConfig->_hostName % status));
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_SESSINFO_SERVER_PORT, connectionConfig->_portNum);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set port");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_OPTION_NET_CONNECTTIMEOUT, 6);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set ssh connection timeout");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    std::string tt("xterm-color");
    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_TERM_TYPE,
                                     tt.c_str(), tt.length());
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set terminal type");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession,
                               CRYPT_SESSINFO_TERM_WIDTH, CB_DEFAULT_TERM_WIDTH);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set terminal width");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttribute(_sshData->_cryptSession,
                               CRYPT_SESSINFO_TERM_HEIGHT, CB_DEFAULT_TERM_HEIGHT);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set terminal height");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    status = cryptSetAttributeString(_sshData->_cryptSession,
                                     CRYPT_SESSINFO_USERNAME,
                                     connectionConfig->_userName.c_str(),
                                     connectionConfig->_userName.length());
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to set username");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }

    bool privateKeyUsed = false;
    /*
     *  To generate PKCS 12 from id_rsa:
     * openssl req -new -x509 -key ~/.ssh/id_rsa -out ssh-cert.pem
     * openssl pkcs12 -export -in ssh-certs.pem -inkey ~/.ssh/id_rsa -out ssh-key.p12
     */
    if (connectionConfig->_privKeyFile.empty() == false)
    {
        privateKeyUsed = tryPrivateKey(connectionConfig);
    }
    if (privateKeyUsed == false)
    {
        status = cryptSetAttributeString(_sshData->_cryptSession,
                                         CRYPT_SESSINFO_PASSWORD,
                                         connectionConfig->_password.c_str(),
                                         connectionConfig->_password.length());
        if (cryptStatusError(status))
        {
            tgtGetErrorMsg(&errmsg, status, "Unable to set password");
            cryptDestroySession(_sshData->_cryptSession);
            throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
        }
    }

    status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_SESSINFO_ACTIVE, true);
    if (cryptStatusError(status))
    {
        tgtGetErrorMsg(&errmsg, status, "Unable to activate session");
        cryptDestroySession(_sshData->_cryptSession);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    else
    {
        _sshData->_sshThread.reset(new boost::thread(boost::bind(&TgtSshIntf::sshThread, this)));
    }
}

bool TgtSshIntf::tryPrivateKey(boost::shared_ptr<const TgtConnectionConfig> connectionConfig)
{
    bool ret = false;
    CRYPT_KEYSET cryptKeyset;
    CRYPT_CONTEXT cryptContext;
    int status;
    std::string errmsg;

    status = cryptKeysetOpen(&cryptKeyset, CRYPT_UNUSED, CRYPT_KEYSET_FILE, connectionConfig->_privKeyFile.c_str(), CRYPT_KEYOPT_READONLY);
    if (cryptStatusError(status) == false)
    {
        status = cryptGetPrivateKey(cryptKeyset, &cryptContext, CRYPT_KEYID_NAME, "[none]", connectionConfig->_password.c_str());
        if (cryptStatusError(status) == false)
        {
            ret = true;
            cryptKeysetClose(cryptKeyset);
            status = cryptSetAttribute(_sshData->_cryptSession, CRYPT_SESSINFO_PRIVATEKEY, cryptContext);
            if (cryptStatusError(status))
            {
                tgtGetErrorMsg(&errmsg, status, "Unable to set private key");
                cryptDestroySession(_sshData->_cryptSession);
                throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
            }
            cryptDestroyContext(cryptContext);
        }
    }

    return ret;
}

int TgtSshIntf::tgtBreakConnection()
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

bool TgtSshIntf::tgtConnected()
{
    return false;
}

void TgtSshIntf::tgtGetTitle(std::string* szTitle)
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
        tgtAttemptReconnect();
    }
}

bool TgtSshIntf::sshSend()
{
    int ret = true;
    boost::intrusive_ptr<RefCntBuffer> b;
    int status;
    int bytesCopied;
    while (_outgoingData.dequeue(b) == true)
    {
        char* data = boost::asio::buffer_cast<char*>(b->_buffer);
        bytesCopied = 0;
        status = cryptPushData(_sshData->_cryptSession, data, boost::asio::buffer_size(b->_buffer), &bytesCopied);
        if (cryptStatusError(status))
        {
            ret = false;
        }
        else if (bytesCopied < (int)boost::asio::buffer_size(b->_buffer))
        {
#ifdef QT_DEBUG
            qDebug("Didnt write everything");
#endif
        }
        else
        {
            status = cryptFlushData(_sshData->_cryptSession);
            if (cryptStatusError(status))
            {
#ifdef QT_DEBUG
                qDebug("Unable to flush data");
#endif
                ret = false;
            }
        }
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
        outDataLength = 0;
        if (_sshData->_currentIncomingBuffer == NULL)
        {
            _bufferPool->dequeue(_sshData->_currentIncomingBuffer, 100);
        }
        if (_sshData->_currentIncomingBuffer != NULL)
        {
            char* data = boost::asio::buffer_cast<char*>(_sshData->_currentIncomingBuffer->_buffer);
            status = cryptPopData(_sshData->_cryptSession, data, boost::asio::buffer_size(_sshData->_currentIncomingBuffer->_buffer), &outDataLength);
            if (cryptStatusError(status))
            {
                ret = false;
            }
            else if (outDataLength > 0)
            {
                _sshData->_currentIncomingBuffer->_buffer = boost::asio::buffer(_sshData->_currentIncomingBuffer->_buffer, outDataLength);
                _incomingData.enqueue(_sshData->_currentIncomingBuffer);
                _sshData->_currentIncomingBuffer.reset();
            }
        }
    } while ((outDataLength > 0) && (cryptStatusOK(status)) && (_sshData->_sshThreadRun == true));
    return ret;
}

