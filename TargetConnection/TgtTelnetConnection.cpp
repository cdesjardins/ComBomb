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

#include "TgtTelnetConnection.h"
#include "CBException.h"
#include <boost/array.hpp>

boost::shared_ptr<TgtTelnetIntf> TgtTelnetIntf::createTelnetConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtTelnetIntf> ret(new TgtTelnetIntf(config));
    ret->tgtAttemptReconnect();
    return ret;
}

TgtTelnetIntf::TgtTelnetIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config),
      _abortConnection(false)
{
    m_nState = TELNET_STATE_DATA;
    m_nCommand = TELNET_CMD_SB;
    m_bEcho = true;
}

TgtTelnetIntf::~TgtTelnetIntf()
{
    _abortConnection = true;
    tgtDisconnect();
}

void TgtTelnetIntf::clearConnectionQueue()
{
    std::vector<boost::system::error_code> trash;
    while (_connectionQueue.dequeue(trash))
    {

    }
}

void TgtTelnetIntf::tgtMakeConnection()
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    std::vector<boost::asio::ip::address> addresses;
    boost::asio::ip::tcp::resolver resolver(_socketService);
    boost::asio::ip::tcp::resolver::query query(connectionConfig->_hostName, "http");
    boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::system::error_code error;

    while (endpointIterator != end)
    {
        endpoint = *endpointIterator;
        addresses.push_back(endpoint.address());
#ifdef QT_DEBUG
        qDebug("addr: %s", endpoint.address().to_string().c_str());
#endif
        endpointIterator++;
    }

    _telnetServiceThread = TgtThread::create(boost::protect(boost::bind(&TgtTelnetIntf::serviceThread, this)));

    for (std::vector<boost::asio::ip::address>::iterator it = addresses.begin(); ((it != addresses.end()) && (_abortConnection == false)); it++)
    {
        endpoint = boost::asio::ip::tcp::endpoint(*it, connectionConfig->_portNum);
        if (_socket != NULL)
        {
            _socket->close();
        }
        _socket.reset(new boost::asio::ip::tcp::socket(_socketService));
        _socket->open(boost::asio::ip::tcp::v4());
        clearConnectionQueue();
        _socket->async_connect(endpoint, boost::bind(
                &TgtTelnetIntf::connectionHandler, this, boost::asio::placeholders::error));
        bool done;
        do
        {
            done = _connectionQueue.dequeue(error, 1);
            if (_abortConnection == true)
            {
                break;
            }
        } while (done == false);

        if (!error)
        {
            break;
        }
    }
    if (error)
    {
        std::string errmsg;
        boost::format f("Unable to connect to '%s:%i' (%d)");
        errmsg = str(f % connectionConfig->_hostName % connectionConfig->_portNum % error);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    _socket->non_blocking(true);

    _telnetWriterThread = TgtThread::create(boost::protect(boost::bind(&TgtTelnetIntf::writerThread, this)));

    boost::system::error_code err;
    _bufferPool->dequeue(_currentIncomingBuffer);
    tgtReadCallback(err, 0);
}

void TgtTelnetIntf::connectionHandler(const boost::system::error_code& ec)
{
    _connectionQueue.enqueue(ec);
}

void TgtTelnetIntf::tgtBreakConnection()
{
    _telnetServiceThread.reset();
    _telnetWriterThread.reset();

    if (_socket != NULL)
    {
        if (_socket->is_open())
        {
            _socket->cancel();
            _socket->close();
        }
        _socket.reset();
    }
}

bool TgtTelnetIntf::writerThread()
{
    boost::intrusive_ptr<RefCntBuffer> b;
    boost::system::error_code ec;
    bool attemptReconnect = false;
    if (_outgoingData.dequeue(b, 100) == true)
    {
        boost::asio::write(*_socket.get(), boost::asio::buffer(b->_buffer), ec);
        if (ec)
        {
            attemptReconnect = true;
        }
    }
    if (attemptReconnect == true)
    {
        tgtAttemptReconnect();
    }
    return true;
}

void TgtTelnetIntf::tgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (!error)
    {
        boost::asio::mutable_buffer buffer;

        if (bytesTransferred > 0)
        {
            if (_currentIncomingBuffer != NULL)
            {
                _currentIncomingBuffer->_buffer = boost::asio::buffer(_currentIncomingBuffer->_buffer, bytesTransferred);
                boost::intrusive_ptr<RefCntBuffer> readData;
                _bufferPool->dequeue(readData, 100);
                if (readData != NULL)
                {
                    int numBytes = tgtTelnetProcessData(readData);
                    if (numBytes > 0)
                    {
                        readData->_buffer = boost::asio::buffer(readData->_buffer, numBytes);
                        _incomingData.enqueue(readData);
                    }
                }
            }
            _bufferPool->dequeue(_currentIncomingBuffer, 100);
        }
        if (_currentIncomingBuffer == NULL)
        {
            // If there are no buffers available then just throw away the next
            // bit if incoming data...
            buffer = boost::asio::buffer(_throwAway, sizeof(_throwAway) - 1);
        }
        else
        {
            buffer = _currentIncomingBuffer->_buffer;
        }
        _socket->async_read_some(boost::asio::buffer(buffer, boost::asio::buffer_size(buffer) - 1),
                                 boost::bind(&TgtTelnetIntf::tgtReadCallback, this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        tgtAttemptReconnect();
    }
}

bool TgtTelnetIntf::serviceThread()
{
    _socketService.reset();
    if (_socketService.poll() == 0)
    {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
    }
    return true;
}

int TgtTelnetIntf::tgtTelnetData(unsigned char cTelnetRx, char* cReadData)
{
    int nRet = 0;
    switch (cTelnetRx)
    {
        case TELNET_CMD_IAC:
            m_nState = TELNET_STATE_COMMAND;
            break;

        default:
            if (m_bEcho)
            {
                *cReadData = cTelnetRx;
            }
            nRet = 1;
            break;
    }
    return nRet;
}

int TgtTelnetIntf::tgtTelnetCommand(eTelnetCommand cTelnetRx)
{
    m_nState = TELNET_STATE_DATA;
    m_nCommand = cTelnetRx;
    switch (cTelnetRx)
    {
        case TELNET_CMD_IAC:
        case TELNET_CMD_SE:
        case TELNET_CMD_NOP:
        case TELNET_CMD_DM:
        case TELNET_CMD_BRK:
        case TELNET_CMD_IP:
        case TELNET_CMD_AO:
        case TELNET_CMD_AYT:
        case TELNET_CMD_EL:
        case TELNET_CMD_GA:
        case TELNET_CMD_EC:
            break;

        case TELNET_CMD_SB:
        case TELNET_CMD_WILL:
        case TELNET_CMD_WONT:
        case TELNET_CMD_DO:
        case TELNET_CMD_DONT:
            m_nState = TELNET_STATE_OPTION;
            break;
    }
    return 0;
}

void TgtTelnetIntf::tgtSendData(const boost::asio::mutable_buffer &buf)
{
    size_t sent;
    size_t totalSent = 0;
    size_t bufferSize = boost::asio::buffer_size(buf);
    boost::asio::mutable_buffer buffer;
    boost::chrono::system_clock::time_point startTime = boost::chrono::system_clock::now();
    do
    {
        buffer = boost::asio::buffer(buf + totalSent);
        sent = _socket->send(boost::asio::buffer(buffer));
        totalSent += sent;
    } while ((totalSent < bufferSize) && (boost::chrono::system_clock::now() < (startTime + boost::chrono::seconds(1))));
}

void TgtTelnetIntf::tgtSendCommand(eTelnetCommand eCmd, eTelnetOption eOpt)
{
    unsigned char sBuffer[3];
    sBuffer[0] = TELNET_CMD_IAC;
    sBuffer[1] = eCmd;
    sBuffer[2] = eOpt;
    tgtSendData(boost::asio::buffer(sBuffer, sizeof(sBuffer)));
}

int TgtTelnetIntf::tgtConfirm(eTelnetOption eOpt)
{
    switch (m_nCommand)
    {
        case TELNET_CMD_WILL:
            tgtSendCommand(TELNET_CMD_DO, eOpt);
            break;

        case TELNET_CMD_WONT:
            tgtSendCommand(TELNET_CMD_DONT, eOpt);
            break;

        case TELNET_CMD_DO:
            tgtSendCommand(TELNET_CMD_WILL, eOpt);
            break;

        case TELNET_CMD_DONT:
            tgtSendCommand(TELNET_CMD_WONT, eOpt);
            break;

        default:
            break;
    }
    return 0;
}

int TgtTelnetIntf::tgtDeny(eTelnetOption eOpt)
{
    switch (m_nCommand)
    {
        case TELNET_CMD_WILL:
            tgtSendCommand(TELNET_CMD_DONT, eOpt);
            break;

        case TELNET_CMD_WONT:
            tgtSendCommand(TELNET_CMD_DO, eOpt);
            break;

        case TELNET_CMD_DO:
            tgtSendCommand(TELNET_CMD_WONT, eOpt);
            break;

        case TELNET_CMD_DONT:
            tgtSendCommand(TELNET_CMD_WILL, eOpt);
            break;

        default:
            break;
    }
    return 0;
}

int TgtTelnetIntf::tgtProcessTerm()
{
    int nReadIndex = 0;
    char sBuffer[] =
    {
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SB,
        (char)TELNET_OPT_TERM,
        0,
        'v', 't', '3', '2', '0',
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SE
    };
    switch (m_nCommand)
    {
        case TELNET_CMD_SB:
            tgtSendData(boost::asio::buffer(sBuffer, sizeof(sBuffer)));
            break;

        case TELNET_CMD_WILL:
            tgtDeny(TELNET_OPT_TERM);
            break;

        case TELNET_CMD_WONT:
            break;

        case TELNET_CMD_DO:
            tgtConfirm(TELNET_OPT_TERM);
            break;

        case TELNET_CMD_DONT:
            break;

        default:
            break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::tgtProcessEcho()
{
    switch (m_nCommand)
    {
        case TELNET_CMD_SB:
            break;

        case TELNET_CMD_WILL:
            m_bEcho = true;
            tgtConfirm(TELNET_OPT_ECHO);
            break;

        case TELNET_CMD_WONT:
            m_bEcho = false;
            tgtConfirm(TELNET_OPT_ECHO);
            break;

        case TELNET_CMD_DO:
            tgtDeny(TELNET_OPT_ECHO);
            break;

        case TELNET_CMD_DONT:
            tgtConfirm(TELNET_OPT_ECHO);
            break;

        default:
            break;
    }
    return 0;
}

int TgtTelnetIntf::tgtProcessUnknownOption(eTelnetOption eOpt)
{
    switch (m_nCommand)
    {
        case TELNET_CMD_WILL:
        case TELNET_CMD_DO:
            tgtDeny(eOpt);
            break;

        default:
            break;
    }
    return 0;
}

int TgtTelnetIntf::tgtTelnetOption(eTelnetOption eOpt)
{
    int nReadIndex = 0;
    m_nState = TELNET_STATE_DATA;
    switch (eOpt)
    {
        case TELNET_OPT_ECHO:
            nReadIndex = tgtProcessEcho();
            break;

        case TELNET_OPT_TERM:
            nReadIndex = tgtProcessTerm();
            break;

        case TELNET_OPT_SUPP:
        case TELNET_OPT_BIN:
        case TELNET_OPT_RECN:
        case TELNET_OPT_APRX:
        case TELNET_OPT_STAT:
        case TELNET_OPT_TIM:
        case TELNET_OPT_REM:
        case TELNET_OPT_OLW:
        case TELNET_OPT_OPS:
        case TELNET_OPT_OCRD:
        case TELNET_OPT_OHT:
        case TELNET_OPT_OHTD:
        case TELNET_OPT_OFD:
        case TELNET_OPT_OVT:
        case TELNET_OPT_OVTD:
        case TELNET_OPT_OLD:
        case TELNET_OPT_EXT:
        case TELNET_OPT_LOGO:
        case TELNET_OPT_BYTE:
        case TELNET_OPT_DATA:
        case TELNET_OPT_SUP:
        case TELNET_OPT_SUPO:
        case TELNET_OPT_SNDL:
        case TELNET_OPT_EOR:
        case TELNET_OPT_TACACS:
        case TELNET_OPT_OM:
        case TELNET_OPT_TLN:
        case TELNET_OPT_3270:
        case TELNET_OPT_X3:
        case TELNET_OPT_NAWS:
        case TELNET_OPT_TS:
        case TELNET_OPT_RFC:
        case TELNET_OPT_LINE:
        case TELNET_OPT_XDL:
        case TELNET_OPT_ENVIR:
        case TELNET_OPT_AUTH:
        case TELNET_OPT_NENVIR:
        case TELNET_OPT_EXTOP:
            tgtProcessUnknownOption(eOpt);
            break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::tgtTelnetProcessData(const boost::intrusive_ptr<RefCntBuffer> &readData)
{
    size_t nNumBytes = boost::asio::buffer_size(_currentIncomingBuffer->_buffer);
    size_t nRxIndex;
    int nReadIndex = 0;
    unsigned char* sTelnetRx = boost::asio::buffer_cast<unsigned char*>(_currentIncomingBuffer->_buffer);
    char* szReadData = boost::asio::buffer_cast<char*>(readData->_buffer);
    for (nRxIndex = 0; nRxIndex < nNumBytes; nRxIndex++)
    {
        switch (m_nState)
        {
            case TELNET_STATE_DATA:
                nReadIndex += tgtTelnetData(sTelnetRx[nRxIndex], &szReadData[nReadIndex]);
                break;

            case TELNET_STATE_COMMAND:
                tgtTelnetCommand((eTelnetCommand)(sTelnetRx[nRxIndex] & 0xFF));
                break;

            case TELNET_STATE_OPTION:
                tgtTelnetOption((eTelnetOption)(sTelnetRx[nRxIndex] & 0xFF));
                if (m_nCommand == TELNET_CMD_SB)
                {
                    while (((sTelnetRx[nRxIndex + 1] & 0xFF) != TELNET_CMD_IAC) && (nRxIndex < nNumBytes))
                    {
                        nRxIndex++;
                    }
                }
                break;
        }
    }
    return nReadIndex;
}

void TgtTelnetIntf::tgtGetTitle(std::string* szTitle)
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    std::stringstream t;
    t << connectionConfig->_hostName << ":" << connectionConfig->_portNum;
    *szTitle = t.str();
}

