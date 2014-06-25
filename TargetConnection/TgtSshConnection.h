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
        TgtConnectionConfig(const std::string &hostName, const int portNum, const std::string &userName, const std::string &password, const std::string &privKeyFile)
            : _hostName(hostName),
            _portNum(portNum),
            _userName(userName),
            _password(password),
            _privKeyFile(privKeyFile)
        {
        }

        TgtConnectionConfig()
        {
        }

        std::string _hostName;
        int _portNum;
        std::string _userName;
        std::string _password;
        std::string _privKeyFile;
    };
    static boost::shared_ptr<TgtSshIntf> createSshConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtSshIntf ();
    virtual int tgtBreakConnection();
    virtual bool tgtConnected();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    bool tryPrivateKey(boost::shared_ptr<const TgtConnectionConfig> connectionConfig);
    void tgtGetErrorMsg(std::string* errmsg, int status, const std::string &defaultErrMsg);
    TgtSshIntf (const boost::shared_ptr<const TgtConnectionConfig> &config);
    void sshThread();
    bool sshRecv();
    bool sshSend();
    virtual void tgtMakeConnection();

    boost::scoped_ptr<TgtSshImpl> _sshData;
};

#endif
