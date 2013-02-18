#include "cbtextedit.h"
#include <QKeyEvent>

static int _colormap[] =
{
    0x00afaf,
    0xaf0000,
    0x00af00,
    0xafaf00,

    0x1f1faf,
    0xaf00af,
    0x00afaf,
    0xafafaf,

    0xffff00,
    0xff0000,
    0x00ff00,
    0xffff00,

    0x1f1fff,
    0xff00ff,
    0x00ffff,
    0xffffff
};

CBTextEdit::CBTextEdit(QWidget *parent)
    :QTextEdit(parent)
{
    _targetInterface = NULL;
    _runThread = true;
    for (int i = 0; i < (sizeof(_colormap) / sizeof(_colormap[0])); i++)
    {
        _colors.push_back(QColor(QRgb(_colormap[i])));
    }
}

CBTextEdit::~CBTextEdit()
{
    _runThread = false;
    _readTargetThread.join();
    _paintScreenThread.join();
}

void CBTextEdit::setTargetInterface(TgtIntf* targetInterface)
{
    _targetInterface = targetInterface;
    _targetInterface->TgtConnect();
    _readTargetThread = boost::thread(&CBTextEdit::readTarget, this);
    _paintScreenThread = boost::thread(&CBTextEdit::paintScreen, this);
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
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

void CBTextEdit::paintChar(int x, int y, char_t *c)
{
    QTextBlockFormat format;
    QString myqdata;
    int fg, bg;

    bg = c->col & 0xf;
    fg = (c->col >> 4) & 0xf;

    if (bg == 0)
    {
        format.setBackground(QColor(QRgb(0x000000)));
    }
    else
    {
        format.setBackground(_colors[bg]);
    }

    if (fg == 7)
    {
        format.setForeground(QColor(QRgb(0xffffff)));
    }
    else
    {
        format.setForeground(_colors[fg]);
    }

    if (c->text == 0)
    {
        //myqdata = " ";
    }
    else
    {
        myqdata = c->text;
        textCursor().setPosition(x + (win->ws_conf.ws_col * y));
        textCursor().insertBlock(format);
        textCursor().insertText(myqdata);
    }
    /*
    XDrawImageString(window.dpy, window.window, window.gc,
                     x * window.font_w,
                     (y * window.font_h) + offset, (char *)data, 1);
    */
}

void CBTextEdit::paintScreen()
{
    int x, y;
    while (_runThread == true)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        if (win->dirty == true)
        {
            for (y = 0; y < win->ws_conf.ws_row; y++)
            {
                for (x = 0; x < win->ws_conf.ws_col; x++)
                {
                    paintChar(x, y, &win->chars[x + (win->ws_conf.ws_col * y)]);
                }
            }

            win->dirty = false;
        }
    }
}
