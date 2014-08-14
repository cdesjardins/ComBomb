#include "QTerminal/QTerminalConfig.h"

QTerminalConfig::QTerminalConfig()
    : _wordSelectionDelimiters("@-./_~")
{
    _font.setStyleHint(QFont::Courier);
}

QDataStream &operator<<(QDataStream &out, const QTerminalConfig &q)
{
    out << q._wordSelectionDelimiters;
    out << q._font;
    return out;
}

QDataStream &operator>>(QDataStream &in, QTerminalConfig &q)
{
    q = QTerminalConfig();
    in >> q._wordSelectionDelimiters;
    in >> q._font;
    return in;
}
