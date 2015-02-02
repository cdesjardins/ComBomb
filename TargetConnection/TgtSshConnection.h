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
        TgtConnectionConfig(const std::string& hostName, const int portNum, const std::string& userName, const std::string& password, const std::string& privKeyFile)
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
    static boost::shared_ptr<TgtSshIntf> createSshConnection(const boost::shared_ptr<const TgtConnectionConfig>& config);
    virtual ~TgtSshIntf ();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    bool tryPrivateKey(boost::shared_ptr<const TgtConnectionConfig> connectionConfig);
    void tgtGetErrorMsg(std::string* errmsg, int status, const std::string& defaultErrMsg);
    TgtSshIntf (const boost::shared_ptr<const TgtConnectionConfig>& config);
    bool sshThread();
    bool sshRecv();
    bool sshSend();
    virtual void tgtBreakConnection();
    virtual void tgtMakeConnection();

    boost::scoped_ptr<TgtSshImpl> _sshData;
};

#endif
