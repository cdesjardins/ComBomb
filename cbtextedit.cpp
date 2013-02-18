#include "cbtextedit.h"
#include <QKeyEvent>

CBTextEdit::CBTextEdit(QWidget *parent)
    :QTextEdit(parent)
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

    for (QString::Iterator it = e->text().begin(); it != e->text().end(); it++)
    {
        if (it->isPrint())
        {
            if (e->modifiers() == Qt::ShiftModifier)
            {
                char byte = it->toUpper().toLatin1();
                byte = toupper(byte);
                if ((byte >= '@') && (byte <= '_'))
                {
                    byte -= 0x40;
                    char_out(byte);
                }
            }
            else
            {
                char_out(it->toLatin1());
            }
        }
        else
        {
            switch (e->key())
            {
            case Qt::Key_Tab:
                char_out('\t');
                break;
            case Qt::Key_Backspace:
                char_out(KEY_BACKSPACE);
                break;
            case Qt::Key_Escape:
                char_out('\33');
                break;
            case Qt::Key_Return:
                char_out('\r');
                break;
            case Qt::Key_Left:
                vt_send(K_LT);
                break;
            case Qt::Key_Right:
                vt_send(K_RT);
                break;
            case Qt::Key_Up:
                vt_send(K_UP);
                break;
            case Qt::Key_Down:
                vt_send(K_DN);
                break;
            case Qt::Key_PageUp:
                vt_send(K_PGUP);
                break;
            case Qt::Key_PageDown:
                vt_send(K_PGDN);
                break;
            case Qt::Key_Home:
                vt_send(K_HOME);
                break;
            case Qt::Key_Insert:
                vt_send(K_INS);
                break;
            case Qt::Key_Delete:
                vt_send(K_DEL);
                break;
            case Qt::Key_End:
                vt_send(K_END);
                break;
            }
        }
    }
}

void CBTextEdit::char_out(char c)
{
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(&c, 1);
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
            for (int i = 0; i < bytes; i++)
            {
                vt_out(data[i]);
            }
        }
    }
}
