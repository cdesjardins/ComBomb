#include "TargetIntf.h"
#include <QDebug>

boost::shared_ptr<TgtSshIntf> TgtSshIntf::createSshConnection(const TgtConnection &config)
{
    boost::shared_ptr<TgtSshIntf> ret(new TgtSshIntf(config));
    ret->_sshThreadRun = true;

    return ret;
}

TgtSshIntf::TgtSshIntf(const TgtConnection &config)
    :_tgtConnectionConfig(config)
{

}

void TgtSshIntf::TgtMakeConnection()
{
    int status;
    status = cryptCreateSession(&_cryptSession, CRYPT_UNUSED, CRYPT_SESSION_SSH);
    if (cryptStatusError(status))
    {
        qDebug("Unable to create SSH session");
    }
    else
    {
        status = cryptSetAttributeString(_cryptSession,
                                         CRYPT_SESSINFO_SERVER_NAME,
                                         _tgtConnectionConfig._hostName.c_str(),
                                         _tgtConnectionConfig._hostName.length());
        if (cryptStatusError(status))
        {
            qDebug("cryptSetAttribute/AttributeString() failed with error code %d, line %d.\n", status, __LINE__);
        }
        else
        {
            status = cryptSetAttribute(_cryptSession, CRYPT_OPTION_NET_CONNECTTIMEOUT, 10);
            if (cryptStatusError(status))
            {
                qDebug("Unable to set connection timeout");
            }
            else
            {
                status = cryptSetAttributeString(_cryptSession,
                                                 CRYPT_SESSINFO_USERNAME,
                                                 _tgtConnectionConfig._userName.c_str(),
                                                 _tgtConnectionConfig._userName.length());
                if (cryptStatusError(status))
                {
                    qDebug("Unable to set username");
                }
                else
                {
                    status = cryptSetAttributeString(_cryptSession,
                                                     CRYPT_SESSINFO_PASSWORD,
                                                     _tgtConnectionConfig._password.c_str(),
                                                     _tgtConnectionConfig._password.length());
                    if (cryptStatusError(status))
                    {
                        qDebug("Unable to set password");
                    }
                    else
                    {
                        status = cryptSetAttribute(_cryptSession, CRYPT_SESSINFO_ACTIVE, true);
                        if (cryptStatusError(status))
                        {
                            qDebug("Unable to activate session");
                        }
                        else
                        {
                            _sshThread.reset(new boost::thread(boost::bind(&TgtSshIntf::sshThread, this)));
                        }
                    }
                }
            }
        }
    }
}

TgtSshIntf::~TgtSshIntf ()
{
    qDebug("Destroyed");
    TgtDisconnect();
}

int TgtSshIntf::TgtDisconnect()
{
    if (_sshThreadRun == true)
    {
        _sshThreadRun = false;
        if (_sshThread->joinable())
        {
            _sshThread->join();
            _sshThread.reset();
        }
    }
    return 0;
}

bool TgtSshIntf::TgtConnected()
{
    return false;
}

void TgtSshIntf::TgtGetTitle(std::string *szTitle)
{
    std::stringstream t;
    t << _tgtConnectionConfig._hostName << ":" << _tgtConnectionConfig._portNum;
    *szTitle = t.str();
}

void TgtSshIntf::sshSend()
{
    boost::asio::mutable_buffer b;
    int status;
    int bytesCopied;
    while (_outgoingData.dequeue(b) == true)
    {
        char *data = boost::asio::buffer_cast<char*>(b);
        bytesCopied = 0;
        status = cryptPushData(_cryptSession, data, boost::asio::buffer_size(b), &bytesCopied);
        if (cryptStatusError(status))
        {
            qDebug("Unable to send data");
        }
        else if (bytesCopied < (int)boost::asio::buffer_size(b))
        {
            qDebug("Didnt write everything");
        }
        else
        {
            status = cryptFlushData(_cryptSession);
            if (cryptStatusError(status))
            {
                qDebug("Unable to flush data");
            }
        }
        TgtReturnReadBuffer(b);
    }
}

void TgtSshIntf::sshThread()
{
    while (_sshThreadRun == true)
    {
        sshSend();
        sshRecv();
    }
    cryptDestroySession(_cryptSession);
    qDebug("ssh done");
}

void TgtSshIntf::sshRecv()
{
    int outDataLength;
    int status;
    do
    {
        char *data = boost::asio::buffer_cast<char*>(_currentIncomingBuffer);
        outDataLength = 0;
        status = cryptPopData(_cryptSession, data, boost::asio::buffer_size(_currentIncomingBuffer), &outDataLength);
        if (cryptStatusError(status))
        {
            qDebug("Unable to recv data");
        }
        else if (outDataLength > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, outDataLength));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
    } while ((outDataLength > 0) && (cryptStatusOK(status)));
}
