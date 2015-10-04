#ifndef CB_TGT_CPPSSH_CONNECTION_H
#define CB_TGT_CPPSSH_CONNECTION_H

#include "../QTerminal/TgtIntf.h"

struct TgtCppsshImpl;

class TgtCppsshIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(const std::string& hostName, const int portNum, const std::string& userName,
                            const std::string& password, const std::string& privKeyFile)
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
    static std::shared_ptr<TgtCppsshIntf> createCppsshConnection(
        const std::shared_ptr<const TgtConnectionConfig>& config);
    virtual ~TgtCppsshIntf ();
    virtual void tgtGetTitle(std::string* szTitle);
    virtual void tgtWindowResize(int cols, int rows);

protected:
    TgtCppsshIntf(const std::shared_ptr<const TgtConnectionConfig>& config);
    bool isConnected();
    bool sshThread();
    bool sshRecv();
    bool sshSend();
    virtual void tgtBreakConnection();
    virtual void tgtMakeConnection();

    std::unique_ptr<TgtCppsshImpl> _sshData;
};

#endif
