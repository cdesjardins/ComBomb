#ifndef QTERMINALCONFIG_H
#define QTERMINALCONFIG_H

#include <string>
#include "unparam.h"

class QTerminalConfig
{
public:
    QTerminalConfig();
    std::string _wordSelectionDelimiters;
};


namespace boost
{
namespace serialization
{
template<class Archive>
void serialize(Archive & ar, QTerminalConfig & config, const unsigned int version)
{
    UNREF_PARAM(version);
    ar & config._wordSelectionDelimiters;
}
}
}

#endif // QTERMINALCONFIG_H
