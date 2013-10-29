#ifndef CB_TGT_SSH_CONNECTION_H
#define CB_TGT_SSH_CONNECTION_H

#include "QTerminal/TgtIntf.h"

#ifndef Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif

struct TgtSshImpl;

class TgtSshIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(const std::string &hostName, const int portNum, const std::string &userName, const std::string &password)
            : _hostName(hostName),
            _portNum(portNum),
            _userName(userName),
            _password(password)
        {
        }

        TgtConnectionConfig()
        {
        }

        std::string _hostName;
        int _portNum;
        std::string _userName;
        std::string _password;
    };
    static boost::shared_ptr<TgtSshIntf> createSshConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtSshIntf ();
    virtual int TgtDisconnect();
    virtual bool TgtConnected();
    virtual void TgtGetTitle(std::string* szTitle);

protected:
    void TgtGetErrorMsg(std::string* errmsg, int status, const std::string &defaultErrMsg);
    TgtSshIntf (const boost::shared_ptr<const TgtConnectionConfig> &config);
    void sshThread();
    bool sshRecv();
    bool sshSend();
    virtual void TgtMakeConnection();

    boost::scoped_ptr<TgtSshImpl> _sshData;
};

#endif
