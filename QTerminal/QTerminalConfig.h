#ifndef QTERMINALCONFIG_H
#define QTERMINALCONFIG_H

#include <QString>
#include <QDataStream>
#include <QFont>
#include "unparam.h"

class QTerminalConfig
{
public:
    QTerminalConfig();
    QString _wordSelectionDelimiters;
    QFont _font;
};

QDataStream &operator<<(QDataStream &out, const QTerminalConfig &q);
QDataStream &operator>>(QDataStream &in, QTerminalConfig &q);

#endif // QTERMINALCONFIG_H
