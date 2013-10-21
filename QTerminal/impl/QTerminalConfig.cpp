#include "QTerminal/QTerminalConfig.h"

QTerminalConfig::QTerminalConfig()
    : _wordSelectionDelimiters("@-./_~")
{

}

QDataStream &operator<<(QDataStream &out, const QTerminalConfig &q)
{
    out << q._wordSelectionDelimiters;
    return out;
}

QDataStream &operator>>(QDataStream &in, QTerminalConfig &q)
{
    q = QTerminalConfig();
    in >> q._wordSelectionDelimiters;
    return in;
}
