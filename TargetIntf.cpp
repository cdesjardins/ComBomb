#include "TargetIntf.h"

/*
** If telnet or ssh to a remote server and it is unix based and you see
** ^H every time you issue a backspace, then add stty erase ^H to your
** startup script (.cshrc) or the like.
*/
#define TGT_BUFFER_SIZE   4096
TgtIntf::TgtIntf(void)
{
    for (size_t i = 0; i < 4096; i++)
    {
        char *buffer = new char[TGT_BUFFER_SIZE];
        _bufferPool.enqueue(boost::asio::mutable_buffer(buffer, TGT_BUFFER_SIZE - 1));
    }

    _bufferPool.dequeue(_currentIncomingBuffer);

    m_nTotalTx = 0;
    m_nTotalRx = 0;
}

TgtIntf::~TgtIntf(void)
{
    _bufferPool.iterate(boost::bind(&TgtIntf::deleteBuffersFunctor, _1));
}

int TgtIntf::deleteBuffersFunctor(std::list<boost::asio::mutable_buffer> &pool)
{
    int ret = pool.size();
    std::list<boost::asio::mutable_buffer>::iterator it;
    for (it = pool.begin(); it != pool.end(); it++)
    {
        char *data = boost::asio::buffer_cast<char*>(*it);
        delete data;
    }
    pool.clear();
    return -ret;
}

void TgtIntf::TgtReturnReadBuffer(const boost::asio::mutable_buffer &b)
{
    _bufferPool.enqueue(boost::asio::buffer(b, TGT_BUFFER_SIZE - 1));
}

int TgtIntf::TgtRead(boost::asio::mutable_buffer &b)
{
    int ret = 0;
    if (_incomingData.waitDequeue(b, 1) == true)
    {
        ret = boost::asio::buffer_size(b);
        char *data = boost::asio::buffer_cast<char*>(b);
        data[ret] = 0;
    }
    return ret;
}

int TgtIntf::TgtWrite(const char *szWriteData, int nBytes)
{
    int ret = 0;
    boost::asio::mutable_buffer b;
    _bufferPool.dequeue(b);
    boost::asio::buffer_copy(b, boost::asio::buffer(szWriteData, nBytes));
    _outgoingData.enqueue(boost::asio::buffer(b, nBytes));
    return ret;
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

boost::shared_ptr<TgtSerialIntf> TgtSerialIntf::createSerialConnection(const TgtConnection &config)
{
    boost::shared_ptr<TgtSerialIntf> ret(new TgtSerialIntf(config));
    ret->_serviceThreadRun = true;
    ret->_serialServiceThread.reset(new boost::thread(boost::bind(&TgtSerialIntf::serviceThread, ret.get())));
    ret->_serialWriterThread.reset(new  boost::thread(boost::bind(&TgtSerialIntf::writerThread, ret.get())));
    return ret;
}

void TgtSerialIntf::serviceThread()
{
    do
    {
        _service.run();
        _service.reset();
    } while (_serviceThreadRun == true);
}

void TgtSerialIntf::writerThread()
{
    boost::asio::mutable_buffer b;
    while (_serviceThreadRun == true)
    {
        if (_outgoingData.dequeue(b) == true)
        {
            //int ret = _port.write_some(boost::asio::buffer(b));
            boost::asio::write(_port, boost::asio::buffer(b));
            TgtReturnReadBuffer(b);
        }
        else
        {
            boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        }
    }
}

TgtSerialIntf::TgtSerialIntf (const TgtConnection &config)
    : _tgtConnectionConfig(config),
      _port(_service, config._portName)
{
    _port.set_option(config._baudRate);
    _port.set_option(config._parity);
    _port.set_option(config._byteSize);
    _port.set_option(config._stopBits);
    _port.set_option(config._flowControl);
}

TgtSerialIntf::~TgtSerialIntf ()
{

}

void TgtSerialIntf::TgtReadCallback(const boost::system::error_code& error, const size_t bytesTransferred)
{
    if (!error)
    {
        if (bytesTransferred > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, bytesTransferred));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
        _port.async_read_some(boost::asio::buffer(_currentIncomingBuffer),
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

int TgtSerialIntf::TgtDisconnect()
{
    if (_serviceThreadRun == true)
    {
        _serviceThreadRun = false;
        _service.stop();
        _serialServiceThread->join();
        _serialWriterThread->join();
    }
    return 0;
}

bool TgtSerialIntf::TgtConnected()
{
    return true;
}

void TgtSerialIntf::TgtGetTitle(std::string *szTitle)
{
    std::string parity;
    std::string stopbits;

    switch (_tgtConnectionConfig._parity.value())
    {
    case boost::asio::serial_port_base::parity::even:
        parity = "e";
        break;
    case boost::asio::serial_port_base::parity::none:
        parity = "n";
        break;
    case boost::asio::serial_port_base::parity::odd:
        parity = "o";
        break;
    }
    switch (_tgtConnectionConfig._stopBits.value())
    {
    case boost::asio::serial_port_base::stop_bits::one:
        stopbits = "1";
        break;
    case boost::asio::serial_port_base::stop_bits::onepointfive:
        stopbits = "1.5";
        break;
    case boost::asio::serial_port_base::stop_bits::two:
        stopbits = "2";
        break;
    }

    std::stringstream t;
    t << _tgtConnectionConfig._portName << " "
      << _tgtConnectionConfig._baudRate.value() << " "
      << parity
      << _tgtConnectionConfig._byteSize.value()
      << stopbits;
    *szTitle = t.str();
}

/******************************************************************************
**
**  File
**
******************************************************************************/
boost::shared_ptr<TgtFileIntf> TgtFileIntf::createFileConnection(const TgtConnection &config)
{
    boost::shared_ptr<TgtFileIntf> ret(new TgtFileIntf(config));
    return ret;
}

TgtFileIntf::TgtFileIntf(const TgtConnection &config)
    : _tgtConnectionConfig(config)
{
}

TgtFileIntf::~TgtFileIntf()
{
}

void TgtFileIntf::TgtMakeConnection()
{
    _inputFile.open(_tgtConnectionConfig._fileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (_inputFile == NULL)
    {
        //throw std::exception(_tgtConnectionConfig._fileName.c_str());
    }
}

int TgtFileIntf::TgtDisconnect()
{
    return 0;
}

int TgtFileIntf::TgtRead(boost::asio::mutable_buffer &b)
{
    size_t ret = 0;
    if (_inputFile)
    {
        char *data = boost::asio::buffer_cast<char*>(_currentIncomingBuffer);
        _inputFile.read(data, boost::asio::buffer_size(_currentIncomingBuffer));
        ret = (size_t)_inputFile.gcount();
        if (ret > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, ret));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
    }
    return TgtIntf::TgtRead(b);
}

bool TgtFileIntf::TgtConnected()
{
    return true;
}

void TgtFileIntf::TgtGetTitle(std::string *szTitle)
{
    *szTitle = _tgtConnectionConfig._fileName;
}
