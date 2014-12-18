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
    ret->tgtMakeConnection();
    return ret;
}

TgtTelnetIntf::TgtTelnetIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config),
    _socket(_socketService)
{
    m_nState = TELNET_STATE_DATA;
    m_nCommand = TELNET_CMD_SB;
    m_bEcho = true;
    m_bConnected = false;
}

TgtTelnetIntf::~TgtTelnetIntf()
{
    m_bConnected = false;
}

void TgtTelnetIntf::tgtMakeConnection()
{
    std::vector<boost::asio::ip::address> addresses;
    boost::asio::ip::tcp::resolver resolver(_socketService);
    boost::asio::ip::tcp::resolver::query query(m_sTgtConnection._hostName, "");
    boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::system::error_code error = boost::asio::error::host_not_found;

    while (endpointIterator != end)
    {
        endpoint = *endpointIterator;
        addresses.push_back(endpoint.address());
        endpointIterator++;
    }

    //addresses.push_back(m_sTgtConnection._hostName);

    for (std::vector<boost::asio::ip::address>::iterator it = addresses.begin(); it != addresses.end(); it++)
    {
        endpoint = boost::asio::ip::tcp::endpoint(*it, m_sTgtConnection._portNum);
        _socket.close();
        _socket.open(boost::asio::ip::tcp::v4());
        _socket.connect(endpoint, error);
        if (!error)
        {
            break;
        }
    }
    if (error)
    {
        std::string errmsg;
        boost::format f("Unable to connect to '%s' (%d)");
        str(f % m_sTgtConnection._hostName % error);
        throw CB_EXCEPTION_STR(CBException::CbExcp, errmsg.c_str());
    }
    _socket.non_blocking(true);
}

int TgtTelnetIntf::tgtBreakConnection()
{
    m_bConnected = false;
    _socket.close();
    return 0;
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
    do
    {
        buffer = boost::asio::buffer(buf + totalSent);
        sent = _socket.send(boost::asio::buffer(buffer));
        totalSent += sent;
        // FIXME: Add timeout
    } while (totalSent < bufferSize);
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

int TgtTelnetIntf::tgtTelnet(char* sTelnetRx, int nNumBytes, char* szReadData)
{
    int nRxIndex;
    int nReadIndex = 0;
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

/*
int TgtTelnetIntf::tgtRead(char* szReadData, int nMaxBytes)
{
    int nNumBytes;

    nNumBytes = _socket->receive(m_sTelnetRx, sizeof(m_sTelnetRx), 0);
    if (nNumBytes > 0)
    {
        //m_nTotalRx += nNumBytes;
    }
    else if ((nNumBytes == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
    {
        void DebugOutput(const char* szFormat, ...);

        DebugOutput("recv error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return TgtTelnet(m_sTelnetRx, nNumBytes, szReadData);
}

int TgtTelnetIntf::TgtWrite(char* szWriteData, int nBytes)
{
    int nNumBytes;
    nNumBytes = send(m_nSocket, szWriteData, nBytes, 0);
    if (nNumBytes > 0)
    {
        //m_nTotalTx += nNumBytes;
    }
    else if (nNumBytes == SOCKET_ERROR)
    {
        void DebugOutput(const char* szFormat, ...);

        DebugOutput("send error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return nNumBytes;
}
*/
bool TgtTelnetIntf::tgtConnected()
{
    return m_bConnected;
}

void TgtTelnetIntf::tgtGetTitle(std::string* szTitle)
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    std::stringstream t;
    t << connectionConfig->_hostName << ":" << connectionConfig->_portNum;
    *szTitle = t.str();
}

