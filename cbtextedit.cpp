#include "cbtextedit.h"
#include "LineFont.h"
#include <QKeyEvent>
#include <QtGui/QPainter>

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
    textCursor().insertBlock();
    setAttribute(Qt::WA_OpaquePaintEvent);
}

CBTextEdit::~CBTextEdit()
{
    _runThread = false;
    _readTargetThread.join();
}

enum LineEncode
{
    TopL  = (1<<1),
    TopC  = (1<<2),
    TopR  = (1<<3),

    LeftT = (1<<5),
    Int11 = (1<<6),
    Int12 = (1<<7),
    Int13 = (1<<8),
    RightT = (1<<9),

    LeftC = (1<<10),
    Int21 = (1<<11),
    Int22 = (1<<12),
    Int23 = (1<<13),
    RightC = (1<<14),

    LeftB = (1<<15),
    Int31 = (1<<16),
    Int32 = (1<<17),
    Int33 = (1<<18),
    RightB = (1<<19),

    BotL  = (1<<21),
    BotC  = (1<<22),
    BotR  = (1<<23)
};

static void drawLineChar(QPainter& paint, int x, int y, int w, int h, uchar code)
{
    //Calculate cell midpoints, end points.
    int cx = x + w/2;
    int cy = y + h/2;
    int ex = x + w - 1;
    int ey = y + h - 1;

    quint32 toDraw = LineChars[code];

    //Top _lines:
    if (toDraw & TopL)
        paint.drawLine(cx-1, y, cx-1, cy-2);
    if (toDraw & TopC)
        paint.drawLine(cx, y, cx, cy-2);
    if (toDraw & TopR)
        paint.drawLine(cx+1, y, cx+1, cy-2);

    //Bot _lines:
    if (toDraw & BotL)
        paint.drawLine(cx-1, cy+2, cx-1, ey);
    if (toDraw & BotC)
        paint.drawLine(cx, cy+2, cx, ey);
    if (toDraw & BotR)
        paint.drawLine(cx+1, cy+2, cx+1, ey);

    //Left _lines:
    if (toDraw & LeftT)
        paint.drawLine(x, cy-1, cx-2, cy-1);
    if (toDraw & LeftC)
        paint.drawLine(x, cy, cx-2, cy);
    if (toDraw & LeftB)
        paint.drawLine(x, cy+1, cx-2, cy+1);

    //Right _lines:
    if (toDraw & RightT)
        paint.drawLine(cx+2, cy-1, ex, cy-1);
    if (toDraw & RightC)
        paint.drawLine(cx+2, cy, ex, cy);
    if (toDraw & RightB)
        paint.drawLine(cx+2, cy+1, ex, cy+1);

    //Intersection points.
    if (toDraw & Int11)
        paint.drawPoint(cx-1, cy-1);
    if (toDraw & Int12)
        paint.drawPoint(cx, cy-1);
    if (toDraw & Int13)
        paint.drawPoint(cx+1, cy-1);

    if (toDraw & Int21)
        paint.drawPoint(cx-1, cy);
    if (toDraw & Int22)
        paint.drawPoint(cx, cy);
    if (toDraw & Int23)
        paint.drawPoint(cx+1, cy);

    if (toDraw & Int31)
        paint.drawPoint(cx-1, cy+1);
    if (toDraw & Int32)
        paint.drawPoint(cx, cy+1);
    if (toDraw & Int33)
        paint.drawPoint(cx+1, cy+1);

}
/*
void CBTextEdit::drawLineCharString(QPainter& painter, int x, int y, const QString& str)
{
        const QPen& currentPen = painter.pen();

        if ( attributes->rendition & RE_BOLD )
        {
            QPen boldPen(currentPen);
            boldPen.setWidth(3);
            painter.setPen( boldPen );
        }

        for (int i=0 ; i < str.length(); i++)
        {
            uchar code = str[i].cell();
            if (LineChars[code])
                drawLineChar(painter, x + (_fontWidth*i), y, _fontWidth, _fontHeight, code);
        }

        painter.setPen( currentPen );
}
*/
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
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

void CBTextEdit::paintChar(int x, int y, char_t *c)
{
    QTextCharFormat charFormat;
    QString myqdata;
    int fg, bg;

    bg = c->col & 0xf;
    fg = (c->col >> 4) & 0xf;

    if (bg == 0)
    {
        charFormat.setBackground(QBrush(QRgb(0x000000)));
    }
    else
    {
        charFormat.setBackground(_colors[bg]);
    }

    if (fg == 7)
    {
        charFormat.setForeground(QColor(QRgb(0xffffff)));
    }
    else
    {
        charFormat.setForeground(_colors[bg]);
    }

    if (c->text == 0)
    {
        //myqdata = " ";
    }
    else
    {
        myqdata = c->text;
        textCursor().setPosition(x + (win->ws_conf.ws_col * y));
        textCursor().setBlockCharFormat(charFormat);
        textCursor().insertText(myqdata);
    }
    /*
    XDrawImageString(window.dpy, window.window, window.gc,
                     x * window.font_w,
                     (y * window.font_h) + offset, (char *)data, 1);
    */
}

void CBTextEdit::paintEvent(QPaintEvent *e)
{
    int x, y;
    if (win->dirty == true)
    {
        QPainter painter(this);
        QPen pen = painter.pen();
        pen.setColor(QColor(QRgb(0xffffff)));
        painter.setPen(QColor(QRgb(0xffffff)));

        for (y = 0; y < win->ws_conf.ws_row; y++)
        {
            for (x = 0; x < win->ws_conf.ws_col; x++)
            {
                uchar code = win->chars[x + (win->ws_conf.ws_col * y)].text;
                if (LineChars[code])
                {
                    drawLineChar(painter, x, y, 8, 12, code);
                }
            }
        }

        win->dirty = false;
    }
}

