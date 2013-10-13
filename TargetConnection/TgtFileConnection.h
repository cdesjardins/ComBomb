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

    virtual int TgtDisconnect();
    virtual int TgtRead(boost::asio::mutable_buffer &b);
    virtual bool TgtConnected();
    virtual void TgtGetTitle(std::string* szTitle);

protected:
    TgtFileIntf(const boost::shared_ptr<const TgtConnectionConfig> &config);
    virtual void TgtMakeConnection();

    std::ifstream _inputFile;
};

namespace boost
{
namespace serialization
{
template<class Archive>
void serialize(Archive & ar, TgtFileIntf::TgtConnectionConfig & config, const unsigned int version)
{
    UNREF_PARAM(version);
    ar & boost::serialization::base_object<TgtIntf::TgtConnectionConfigBase>(config);
    ar & config._fileName;
}
}
}

#endif
