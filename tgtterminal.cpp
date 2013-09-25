#include "tgtterminal.h"

TgtTerminal::TgtTerminal(const boost::shared_ptr<TgtIntf> &tgtIntf, int w, int h)
    : Terminal(w, h),
      _targetInterface(tgtIntf)
{
}

void TgtTerminal::char_out(char c)
{
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(&c, 1);
    }
}
