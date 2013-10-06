

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

char* TgtTelnetIntf::TgtMakeConnection()
{
    char* pRet;
    struct hostent* pHostEnt;
    struct protoent* pProtEnt;
    struct sockaddr_in sin;
    unsigned long nNonBlocking = 1;

    pRet = NULL;
    memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)m_sTgtConnection.m_nPort);
    pHostEnt = gethostbyname(m_sTgtConnection.m_szServerName);
    if (pHostEnt != 0)
    {
        memcpy((char*)&sin.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);
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
            if (connect(m_nSocket, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
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
    switch (m_nCommand)
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
    switch (m_nCommand)
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
        'v', 't', '3', '2', '0',
        (char)TELNET_CMD_IAC,
        (char)TELNET_CMD_SE
    };
    switch (m_nCommand)
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
    switch (m_nCommand)
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
    switch (m_nCommand)
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
        case TELNET_OPT_ECHO:
            nReadIndex = TgtProcessEcho();
            break;
        case TELNET_OPT_TERM:
            nReadIndex = TgtProcessTerm();
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
            TgtProcessUnknownOption(eOpt);
            break;
    }
    return nReadIndex;
}

int TgtTelnetIntf::TgtTelnet(char* sTelnetRx, int nNumBytes, char* szReadData)
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

int TgtTelnetIntf::TgtRead(char* szReadData, int nMaxBytes)
{
    int nNumBytes;

    nNumBytes = recv(m_nSocket, m_sTelnetRx, sizeof(m_sTelnetRx), 0);
    if (nNumBytes > 0)
    {
        m_nTotalRx += nNumBytes;
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
        m_nTotalTx += nNumBytes;
    }
    else if (nNumBytes == SOCKET_ERROR)
    {
        void DebugOutput(const char* szFormat, ...);

        DebugOutput("send error: %i\n", WSAGetLastError());
        m_bConnected = false;
    }
    return nNumBytes;
}

bool TgtTelnetIntf::TgtConnected()
{
    return m_bConnected;
}

void TgtTelnetIntf::TgtGetTitle(char* szTitle)
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
