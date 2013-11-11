#include "TgtFileConnection.h"
#include "CBException.h"
#include <boost/format.hpp>

/******************************************************************************
**
**  File
**
******************************************************************************/
boost::shared_ptr<TgtFileIntf> TgtFileIntf::createFileConnection(const boost::shared_ptr<const TgtConnectionConfig> &config)
{
    boost::shared_ptr<TgtFileIntf> ret(new TgtFileIntf(config));
    ret->tgtMakeConnection();
    return ret;
}

TgtFileIntf::TgtFileIntf(const boost::shared_ptr<const TgtConnectionConfig> &config)
    : TgtIntf(config)
{
}

void TgtFileIntf::tgtMakeConnection()
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    _inputFile.open(connectionConfig->_fileName.c_str(), std::ifstream::in | std::ifstream::binary);
    if (_inputFile == NULL)
    {
        boost::format f("Unable to open file: %s");
        throw CB_EXCEPTION_STR(CBException::CbExcp, str(f % connectionConfig->_fileName).c_str());
    }
}

TgtFileIntf::~TgtFileIntf()
{
}

int TgtFileIntf::tgtBreakConnection()
{
    return 0;
}

int TgtFileIntf::tgtRead(boost::asio::mutable_buffer &b)
{
    size_t ret = 0;
    if (_inputFile)
    {
        char* data = boost::asio::buffer_cast<char*>(_currentIncomingBuffer);
        _inputFile.read(data, boost::asio::buffer_size(_currentIncomingBuffer));
        ret = (size_t)_inputFile.gcount();
        if (ret > 0)
        {
            _incomingData.enqueue(boost::asio::buffer(_currentIncomingBuffer, ret));
            _bufferPool.dequeue(_currentIncomingBuffer);
        }
    }
    return TgtIntf::tgtRead(b);
}

bool TgtFileIntf::tgtConnected()
{
    return true;
}

void TgtFileIntf::tgtGetTitle(std::string* szTitle)
{
    boost::shared_ptr<const TgtConnectionConfig> connectionConfig = boost::dynamic_pointer_cast<const TgtConnectionConfig>(_connectionConfig);
    *szTitle = connectionConfig->_fileName;
}

