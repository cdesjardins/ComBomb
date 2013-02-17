#include "cbtextedit.h"
#include <QKeyEvent>

CBTextEdit::CBTextEdit(QWidget *parent):QTextEdit(parent)
{
    _targetInterface = NULL;
    _runThread = true;
}

CBTextEdit::~CBTextEdit()
{
    _runThread = false;
    _readTargetThread.join();
}

void CBTextEdit::setTargetInterface(TgtIntf* targetInterface)
{
    _targetInterface = targetInterface;
    _targetInterface->TgtConnect();
    _readTargetThread = boost::thread(&CBTextEdit::readTarget, this);
}

void CBTextEdit::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);
    QByteArray ba = e->text().toLocal8Bit();
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(ba.data(), ba.count());
    }
}

void CBTextEdit::readTarget()
{
    char data[255];
    while (_runThread == true)
    {
        int bytes = _targetInterface->TgtRead(data, sizeof(data));
        if (bytes > 0)
        {
            data[bytes] = NULL;
            append(data);
        }
    }
}
