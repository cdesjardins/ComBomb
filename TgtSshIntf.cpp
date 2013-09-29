#include "TargetIntf.h"


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
