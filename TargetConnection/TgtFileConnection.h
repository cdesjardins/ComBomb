#ifndef CB_TGT_FILE_CONNECTION_H
#define CB_TGT_FILE_CONNECTION_H

#include "QTerminal/TgtIntf.h"
#include <fstream>

class TgtFileIntf : public TgtIntf
{
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(std::string fileName)
            : _fileName(fileName)
        {
        }

        TgtConnectionConfig()
        {
        }

        std::string _fileName;
    };

    static boost::shared_ptr<TgtFileIntf> createFileConnection(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual ~TgtFileIntf(void);

    virtual int tgtBreakConnection();
    virtual int tgtRead(boost::asio::mutable_buffer &b);
    virtual bool tgtConnected();
    virtual void tgtGetTitle(std::string* szTitle);

protected:
    TgtFileIntf(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual void tgtMakeConnection();

    std::ifstream _inputFile;
};

#endif
