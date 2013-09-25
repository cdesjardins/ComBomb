#ifndef TGTTERMINAL_H
#define TGTTERMINAL_H

#include "TargetIntf.h"
#include "vt100.h"
class TgtTerminal : public Terminal
{
public:
    TgtTerminal(const boost::shared_ptr<TgtIntf> &tgtIntf, int w, int h);
    virtual void TgtTerminal::char_out(char c);

    boost::shared_ptr<TgtIntf> _targetInterface;
protected:
};

#endif // TGTTERMINAL_H
