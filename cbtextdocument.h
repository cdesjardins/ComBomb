#ifndef CBTEXTDOCUMENT_H
#define CBTEXTDOCUMENT_H

#include <QTextDocument>
#include "tgtterminal.h"
class CBTextDocument : public QTextDocument
{
public:
    CBTextDocument(const boost::shared_ptr<TgtIntf> &tgtIntf);
    TgtTerminal _tgtTerminal;
};

#endif // CBTEXTDOCUMENT_H
