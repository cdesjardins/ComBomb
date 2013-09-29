#include "TargetIntf.h"
#include <QDebug>

boost::shared_ptr<TgtSshIntf> TgtSshIntf::createSshConnection(const TgtConnection &config)
{
    boost::shared_ptr<TgtSshIntf> ret(new TgtSshIntf(config));
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
    }
}

TgtSshIntf::~TgtSshIntf ()
{

}

int TgtSshIntf::TgtDisconnect()
{
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
