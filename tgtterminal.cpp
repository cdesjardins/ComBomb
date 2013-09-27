#include "tgtterminal.h"

TgtTerminal::TgtTerminal(const boost::shared_ptr<TgtIntf> &tgtIntf, int w, int h)
    : Terminal(w, h),
      _targetInterface(tgtIntf)
{
}

void TgtTerminal::char_out(const char c)
{
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(&c, 1);
    }
}

void TgtTerminal::str_out(const char *s)
{
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(s, strlen(s));
    }
}
