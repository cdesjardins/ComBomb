
#include "TargetIntf.h"
/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/

#define TGT_BACKSPACE       0x08

TgtIntf::TgtIntf(void)
{
    for (int i = 0; i < sizeof(_buffer) / sizeof(_buffer[0]); i++)
    {
        _buffer[i] = 0;
    }
    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
}

/******************************************************************************
**
**  Telnet
**
******************************************************************************/
#if 0
TgtTelnetIntf::TgtTelnetIntf()
{
    m_nSocket = -1;
    m_nState = TELNET_STATE_DATA;
    m_nCommand = TELNET_CMD_SB;
    m_bEcho = true;
    m_bConnected = false;
}

TgtTelnetIntf::~TgtTelnetIntf()
{
    m_bConnected = false;
}

char * TgtTelnetIntf::TgtMakeConnection()
{
    char *pRet;
    struct hostent  *pHostEnt;
    struct protoent *pProtEnt;
    struct sockaddr_in sin;
    unsigned long nNonBlocking = 1;

    pRet = NULL;
    memset((char *)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)m_sTgtConnection.m_nPort);
    pHostEnt = gethostbyname(m_sTgtConnection.m_szServerName);
    if (pHostEnt != 0)
    {
        memcpy((char *)&sin.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);
    }
    else
    {
        sin.sin_addr.s_addr = inet_addr(m_sTgtConnection.m_szServerName);
        if (sin.sin_addr.s_addr == INADDR_NONE)
        {
            pRet = "Unable to find server";
        }
    }
    pProtEnt = getprotobyname("tcp");
    if (pProtEnt == 0)
    {
        pRet = "TCP failure";
    }
    else
    {
        m_nSocket = (int)socket(PF_INET, SOCK_STREAM, pProtEnt->p_proto);
        if (m_nSocket < 0)
        {
            pRet = "Unable to open socket";
        }
        else
        {
            if (connect(m_nSocket, (struct sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
            {
                pRet = "Unable to connect to server";
            }
            else
            {
                if (ioctlsocket(m_nSocket, FIONBIO, &nNonBlocking) == SOCKET_ERROR)
                {
                    pRet = "Socket failure";
                }
                else
                {
                    m_bConnected = true;
                }
            }
        }
    }
    return pRet;
}

int TgtTelnetIntf::TgtDisconnect()
{
    m_bConnected = false;
    if (m_nSocket > 0)
    {
        closesocket(m_nSocket);
    }
    return 0;
}

int TgtTelnetIntf::TgtTelnetData(unsigned char cTelnetRx, char* cReadData)
{
    int nRet = 0;
    switch(cTelnetRx)
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

int TgtTelnetIntf::TgtTelnetCommand(eTelnetCommand cTelnetRx)
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

int TgtTelnetIntf::TgtSendCommand(eTelnetCommand eCmd, eTelnetOption eOpt)
{
    unsigned char sBuffer[3];
    sBuffer[0] = TELNET_CMD_IAC;
    sBuffer[1] = eCmd;
    sBuffer[2] = eOpt;
    return send(m_nSocket, (char*)sBuffer, sizeof(sBuffer), 0);
}

int TgtTelnetIntf::TgtConfirm(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
        TgtSendCommand(TELNET_CMD_DO, eOpt);
        break;
    case TELNET_CMD_WONT:
        TgtSendCommand(TELNET_CMD_DONT, eOpt);
        break;
    case TELNET_CMD_DO:
        TgtSendCommand(TELNET_CMD_WILL, eOpt);
        break;
    case TELNET_CMD_DONT:
        TgtSendCommand(TELNET_CMD_WONT, eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtDeny(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
        TgtSendCommand(TELNET_CMD_DONT, eOpt);
        break;
    case TELNET_CMD_WONT:
        TgtSendCommand(TELNET_CMD_DO, eOpt);
        break;
    case TELNET_CMD_DO:
        TgtSendCommand(TELNET_CMD_WONT, eOpt);
        break;
    case TELNET_CMD_DONT:
        TgtSendCommand(TELNET_CMD_WILL, eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtProcessTerm()
{
    int nReadIndex = 0;
    char sBuffer[] =
    {
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SB,
        (char)TELNET_OPT_TERM,
        0,
        'v','t','3','2','0',
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SE
    };
    switch(m_nCommand)
    {
    case TELNET_CMD_SB:
        send(m_nSocket, (char*)sBuffer, sizeof(sBuffer), 0);
        break;
    case TELNET_CMD_WILL:
        TgtDeny(TELNET_OPT_TERM);
        break;
    case TELNET_CMD_WONT:
        break;
    case TELNET_CMD_DO:
        TgtConfirm(TELNET_OPT_TERM);
        break;
    case TELNET_CMD_DONT:
        break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtProcessEcho()
{
    switch(m_nCommand)
    {
    case TELNET_CMD_SB:
        break;
    case TELNET_CMD_WILL:
        m_bEcho = true;
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_WONT:
        m_bEcho = false;
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_DO:
        TgtDeny(TELNET_OPT_ECHO);
        break;
    case TELNET_CMD_DONT:
        TgtConfirm(TELNET_OPT_ECHO);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtProcessUnknownOption(eTelnetOption eOpt)
{
    switch(m_nCommand)
    {
    case TELNET_CMD_WILL:
    case TELNET_CMD_DO:
        TgtDeny(eOpt);
        break;
    }
    return 0;
}

int TgtTelnetIntf::TgtTelnetOption(eTelnetOption eOpt)
{
    int nReadIndex = 0;
    m_nState = TELNET_STATE_DATA;
    switch (eOpt)
    {
    case TELNET_OPT_ECHO   :
        nReadIndex = TgtProcessEcho();
        break;
    case TELNET_OPT_TERM   :
        nReadIndex = TgtProcessTerm();
        break;
    case TELNET_OPT_SUPP   :
    case TELNET_OPT_BIN    :
    case TELNET_OPT_RECN   :
    case TELNET_OPT_APRX   :
    case TELNET_OPT_STAT   :
    case TELNET_OPT_TIM    :
    case TELNET_OPT_REM    :
    case TELNET_OPT_OLW    :
    case TELNET_OPT_OPS    :
    case TELNET_OPT_OCRD   :
    case TELNET_OPT_OHT    :
    case TELNET_OPT_OHTD   :
    case TELNET_OPT_OFD    :
    case TELNET_OPT_OVT    :
    case TELNET_OPT_OVTD   :
    case TELNET_OPT_OLD    :
    case TELNET_OPT_EXT    :
    case TELNET_OPT_LOGO   :
    case TELNET_OPT_BYTE   :
    case TELNET_OPT_DATA   :
    case TELNET_OPT_SUP    :
    case TELNET_OPT_SUPO   :
    case TELNET_OPT_SNDL   :
    case TELNET_OPT_EOR    :
    case TELNET_OPT_TACACS :
    case TELNET_OPT_OM     :
    case TELNET_OPT_TLN    :
    case TELNET_OPT_3270   :
    case TELNET_OPT_X3     :
    case TELNET_OPT_NAWS   :
    case TELNET_OPT_TS     :
    case TELNET_OPT_RFC    :
    case TELNET_OPT_LINE   :
    case TELNET_OPT_XDL    :
    case TELNET_OPT_ENVIR  :
    case TELNET_OPT_AUTH   :
    case TELNET_OPT_NENVIR :
    case TELNET_OPT_EXTOP  :
        TgtProcessUnknownOption(eOpt);
        break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtTelnet(char *sTelnetRx, int nNumBytes, char *szReadData)
{
    int nRxIndex;
    int nReadIndex = 0;
    for (nRxIndex = 0; nRxIndex < nNumBytes; nRxIndex++)
    {
        switch (m_nState)
        {
        case TELNET_STATE_DATA:
            nReadIndex += TgtTelnetData(sTelnetRx[nRxIndex], &szReadData[nReadIndex]);
            break;
        case TELNET_STATE_COMMAND:
            TgtTelnetCommand((eTelnetCommand)(sTelnetRx[nRxIndex] & 0xFF));
            break;
        case TELNET_STATE_OPTION:
            TgtTelnetOption((eTelnetOption)(sTelnetRx[nRxIndex] & 0xFF));
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

int TgtTelnetIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    int nNumBytes;

    nNumBytes = recv(m_nSocket, m_sTelnetRx, sizeof(m_sTelnetRx), 0);
    if (nNumBytes > 0)
    {
        m_nTotalRx += nNumBytes;
    }
    else if ((nNumBytes == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
    {
        void DebugOutput(const char *szFormat, ...);

        DebugOutput("recv error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return TgtTelnet(m_sTelnetRx, nNumBytes, szReadData);
}

int TgtTelnetIntf::TgtWrite(char *szWriteData, int nBytes)
{
    int nNumBytes;
    nNumBytes = send(m_nSocket, szWriteData, nBytes, 0);
    if (nNumBytes > 0)
    {
        m_nTotalTx += nNumBytes;
    }
    else if (nNumBytes == SOCKET_ERROR)
    {
        void DebugOutput(const char *szFormat, ...);

        DebugOutput("send error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return nNumBytes;
}

bool TgtTelnetIntf::TgtConnected()
{
    return m_bConnected;
}

void TgtTelnetIntf::TgtGetTitle(char *szTitle)
{
    if (_stricmp(m_sTgtConnection.m_szServerName, m_sTgtConnection.m_szDescription) == 0)
    {
        sprintf(szTitle, "Telnet %s %i",
            m_sTgtConnection.m_szDescription,
            m_sTgtConnection.m_nPort);
    }
    else
    {
        sprintf(szTitle, "Telnet %s - %s %i",
            m_sTgtConnection.m_szDescription,
            m_sTgtConnection.m_szServerName,
            m_sTgtConnection.m_nPort);
    }
}
#endif
/******************************************************************************
**
**  Serial
**
******************************************************************************/
#if 1

boost::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(TgtConnection config)
{
    boost::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->_serialThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &ret->_service)));
    return ret;
}

TgtSerialIntf::TgtSerialIntf (TgtConnection config)
    : _tgtConnectionConfig(config),
      _port(_service, config._portName),
      _serialThreadRun(true)
{
    _port.set_option(boost::asio::serial_port_base::baud_rate(115200));
    _port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    _port.set_option(boost::asio::serial_port_base::character_size(8));
    _port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    _port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
}

TgtSerialIntf::~TgtSerialIntf ()
{
}

char * TgtSerialIntf::TgtSetupPort()
{
    char *pRet = "Not implemented";
    /*
    DCB dDcb;

    pRet = NULL;

    memset(&dDcb, 0, sizeof(DCB));
    dDcb.DCBlength = sizeof(DCB);
    if (!GetCommState(m_hSerial, &dDcb))
    {
        pRet = "GetCommState failed";
    }
    else
    {
        dDcb.BaudRate = m_sTgtConnection.m_dwBaudRate;
        dDcb.ByteSize = m_sTgtConnection.m_byByteSize;
        dDcb.Parity = m_sTgtConnection.m_byParity;
        dDcb.StopBits = m_sTgtConnection.m_byStopBits;
        dDcb.fDsrSensitivity = 0;
        dDcb.fDtrControl = DTR_CONTROL_ENABLE;
        dDcb.fOutxDsrFlow = 0;
        if (!SetCommState(m_hSerial, &dDcb))
        {
            pRet = "SetCommState failed";
        }
    }
    */
    return pRet;
}

void TgtSerialIntf::TgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (!error)
    {
        if (bytesTransferred > 0)
        {
            _incomingData.push_back(boost::asio::buffer(_buffer, bytesTransferred));
        }
        _port.async_read_some(boost::asio::buffer(_buffer),
            boost::bind(&TgtSerialIntf::TgtReadCallback, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
}

void TgtSerialIntf::TgtMakeConnection()
{
    boost::system::error_code err;
    TgtReadCallback(err, 0);
}

void TgtSerialIntf::TgtWritePortData(char *szData, int nBytes)
{
    /*
    DWORD dwTotalBytesWrote;
    DWORD dwBytesWrote;
    OVERLAPPED ovWrite;
    DWORD dwMask;
    BOOL bRet;
    HANDLE arHandles[2];
    DWORD dwErrors;

    dwTotalBytesWrote = 0;
    do
    {
        GetCommMask(m_hSerial, &dwMask);

        if (dwMask & EV_TXEMPTY)
        {
            memset(&ovWrite, 0, sizeof(ovWrite));
            ovWrite.hEvent = CreateEvent(0, true, 0, 0);
            bRet = WriteFile(m_hSerial, szData, nBytes, &dwBytesWrote, &ovWrite);
            if (!bRet)
            {
                arHandles[0] = m_hThreadTerm;
                arHandles[1] = ovWrite.hEvent;
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    switch (WaitForMultipleObjects(2, arHandles, FALSE, 1000))
                    {
                    case WAIT_OBJECT_0:
                        _endthread();
                        break;
                    case WAIT_OBJECT_0 + 1:
                        GetOverlappedResult(m_hSerial, &ovWrite, &dwBytesWrote, FALSE);
                        dwTotalBytesWrote += dwBytesWrote;
                        break;
                    }
                }
                else
                {
                    ClearCommError(m_hSerial, &dwErrors, NULL);
                    dwBytesWrote = 0;
                }
            }
            else
            {
                dwTotalBytesWrote += dwBytesWrote;
            }
            CloseHandle(ovWrite.hEvent);
        }
        else
        {
            Sleep(1);
        }
    } while (dwTotalBytesWrote < nBytes);
    */
}

void TgtSerialIntf::TgtSendToPort()
{
    /*
    char szData[64];
    DWORD nBytes = 0;
    do
    {
        EnterCriticalSection(&m_csOutProtector);
        nBytes = Vec2Char(szData, &m_dataOutgoing, sizeof(szData));
        LeaveCriticalSection(&m_csOutProtector);
        TgtWritePortData(szData, nBytes);
        m_nTotalTx += nBytes;
    } while (m_dataOutgoing.size() > 0);
    */
}

void TgtSerialIntf::TgtReadFromPort()
{
    /*
    OVERLAPPED ovRead;
    DWORD dwBytesRead;
    DWORD dwErrors;
    char buf;
    BOOL bRet;

    memset(&ovRead, 0, sizeof(ovRead));
    ovRead.hEvent = CreateEvent(0, true, 0, 0);

    do
    {
        ResetEvent(ovRead.hEvent);
        bRet = ReadFile(m_hSerial, &buf, sizeof(buf), &dwBytesRead, &ovRead);
        if (dwBytesRead > 0)
        {
            EnterCriticalSection(&m_csInProtector);
            m_dataIncoming.push_back(buf);
            LeaveCriticalSection(&m_csInProtector);
        }
        m_nTotalRx += dwBytesRead;
    } while ((bRet) && (dwBytesRead));
    if (!bRet)
    {
        ClearCommError(m_hSerial, &dwErrors, NULL);
    }
    CloseHandle(ovRead.hEvent);
    */
}

int TgtSerialIntf::TgtDisconnect()
{
    /*
    if (m_hThreadTerm)
    {
        SignalObjectAndWait(m_hThreadTerm, m_hSerialMonitor, 1000, FALSE);
        CloseHandle(m_hThreadTerm);
    }
    if (m_hOutput)
    {
        CloseHandle(m_hOutput);
    }
    if (m_hSerial)
    {
        CloseHandle(m_hSerial);
    }
    */
    return 0;
}

int TgtSerialIntf::TgtRead(boost::asio::mutable_buffer &b)
{
    int nRet = 0;
    if (_incomingData.size() > 0)
    {
        b = _incomingData.front();
        _incomingData.pop_front();
    }
    return nRet;
}

int TgtSerialIntf::TgtWrite(char *szWriteData, int nBytes)
{
    int ret = _port.write_some(boost::asio::buffer(szWriteData, nBytes));
    /*
    int nIndex;
    EnterCriticalSection(&m_csOutProtector);
    for (nIndex = 0; nIndex < nBytes; nIndex++)
    {
        m_dataOutgoing.push_back(szWriteData[nIndex]);
    }
    LeaveCriticalSection(&m_csOutProtector);
    SetEvent(m_hOutput);
    */
    return ret;
}

bool TgtSerialIntf::TgtConnected()
{
    return true;
}

void TgtSerialIntf::TgtGetTitle(std::string *szTitle)
{
    /*
    char parity;
    switch (m_sTgtConnection.m_byParity)
    {
    case NOPARITY:
        parity = 'N';
        break;
    case ODDPARITY:
        parity = 'O';
        break;
    case EVENPARITY:
        parity = 'E';
        break;
    case MARKPARITY:
        parity = 'M';
        break;
    case SPACEPARITY:
        parity = 'S';
        break;
    }
    sprintf(szTitle, "SERIAL %s %i %c-%i-%i",
        m_sTgtConnection.m_szPortName,
        m_sTgtConnection.m_dwBaudRate,
        parity, m_sTgtConnection.m_byByteSize,
        m_sTgtConnection.m_byStopBits + 1);
    */
}
#endif

/******************************************************************************
**
**  File
**
******************************************************************************/


TgtFileIntf::TgtFileIntf ()
{
    m_cnt = 0;
}

TgtFileIntf::~TgtFileIntf ()
{
}

void TgtFileIntf::TgtMakeConnection()
{
    m_fInput.open(m_sTgtConnection.m_szFileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (m_fInput == NULL)
    {
        throw std::exception(m_sTgtConnection.m_szFileName.c_str());
    }
}

int TgtFileIntf::TgtDisconnect()
{
    return 0;
}

int TgtFileIntf::TgtRead(char *szReadData, int nMaxBytes)
{
    int nLen = 0;
    m_fInput.read((char*)&nLen, sizeof(int));
    if (m_fInput)
    {
        m_fInput.read(szReadData, nLen);
        return nLen;
    }
    return 0;
}

int TgtFileIntf::TgtWrite(char *szWriteData, int nBytes)
{
    return 0;
}

bool TgtFileIntf::TgtConnected()
{
    return true;
}

void TgtFileIntf::TgtGetTitle(std::string *szTitle)
{
    *szTitle = m_sTgtConnection.m_szFileName;
}
