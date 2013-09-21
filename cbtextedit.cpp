#include "cbtextedit.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QtGui/QPainter>


static int _colormap[] =
{
    0x000000,
    0xaf0000,
    0x00af00,
    0xafaf00,

    0x1f1faf,
    0xaf00af,
    0x00afaf,
    0xafafaf,

    0x1f1f1f,
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

void CBTextEdit::setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface)
{
    _targetInterface = targetInterface;
    _targetInterface->TgtConnect();
    _readTargetThread = boost::thread(&CBTextEdit::readTarget, this);

}

void CBTextEdit::keyPressEvent(QKeyEvent *e)
{
    QChar ch = e->text()[0];

    if (ch.isPrint())
    {
        vt_send(ch.toLatin1());
    }
    else
    {
        switch (e->key())
        {
        case Qt::Key_Tab:
            vt_send('\t');
            break;
        case Qt::Key_Backspace:
            vt_send(KEY_BACKSPACE);
            break;
        case Qt::Key_Escape:
            vt_send('\33');
            break;
        case Qt::Key_Return:
            vt_send('\r');
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

void CBTextEdit::char_out(char c)
{
    if (_targetInterface != NULL)
    {
        _targetInterface->TgtWrite(&c, 1);
    }
}

void CBTextEdit::readTarget()
{
    char buffer[128];
    boost::asio::mutable_buffer b;
    while (_runThread == true)
    {
        b = boost::asio::buffer(buffer);
        int bytes = _targetInterface->TgtRead(b);
        if (bytes > 0)
        {
            char *data = boost::asio::buffer_cast<char*>(b);
            for (int i = 0; i < bytes; i++)
            {
                vt_out(data[i]);
            }
            b = boost::asio::mutable_buffer();
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}

bool CBTextEdit::setCharColor(int *fg, int *bg, char_t *c, QPen *curPen)
{
    bool ret = false;
    int newFg;
    int newBg;

    newBg = c->col & 0xf;
    newFg = (c->col >> 4) & 0xf;

    if (newBg != *bg)
    {
        ret = true;
    }

    if (newFg != *fg)
    {
        curPen->setColor(_colors[newFg]);
        ret = true;
    }
    *fg = newFg;
    *bg = newBg;
    return ret;
}

void CBTextEdit::paintEvent(QPaintEvent *)
{
    int x = 0;
    int y = 0;
    int px = 0;
    int py = 0;
    int fg = -1;
    int bg = -1;
    QPen curPen;
    QString text;
    QPainter painter(this->viewport());
    QFont f = painter.font();
    QFontMetrics fm(f);
    bool draw = false;
    curPen = painter.pen();
    setCharColor(&fg, &bg, &win->chars[0], &curPen);
    painter.setPen(curPen);

    for (y = 0; y < win->ws_conf.ws_row; y++)
    {
        px = 0;
        py += fm.xHeight();
        draw = false;
        for (x = 0; x < win->ws_conf.ws_col; x++)
        {
            char_t *c = &(win->chars[x + (win->ws_conf.ws_col * y)]);
            uchar code = c->text;

            if (code != 0)
            {
                draw = setCharColor(&fg, &bg, c, &curPen);
            }
            else
            {
                draw = true;
            }

            if ((text.length() > 0) && (draw == true))
            {
                QRect textRect = fm.boundingRect(text);
                textRect.moveBottomLeft(QPoint(px - 1, py + 1));
                painter.fillRect(textRect, QBrush(_colors[bg]));
                painter.drawText(QPoint(px, py), text);
                px += fm.width(text);
                text.clear();
                draw = false;
                painter.setPen(curPen);
            }

            if (code != 0)
            {
                text.append(code);
            }
            else
            {
                px += fm.width(" ");
            }
        }
    }
    viewport()->update();
    win->dirty = false;
}

QSize CBTextEdit::sizeHint() const
{
    int width = 80 * fontMetrics().width('x');
    int height = 25 * fontMetrics().lineSpacing();
    QScrollBar *q = verticalScrollBar();
    if (q)
    {
        width += q->width();
    }
    q = horizontalScrollBar();
    if (q)
    {
        height += q->height();
    }
    return QSize(width, height);
}
