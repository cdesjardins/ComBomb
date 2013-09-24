#include "cbtextedit.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QtGui/QPainter>
#include <boost/asio/serial_port.hpp>

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
    :QPlainTextEdit(parent),
      _runThread(true)
{
    qDebug("CBTextEdit");
    for (size_t i = 0; i < (sizeof(_colormap) / sizeof(_colormap[0])); i++)
    {
        _colors.push_back(QColor(QRgb(_colormap[i])));
    }
    textCursor().insertBlock();
    setAttribute(Qt::WA_OpaquePaintEvent);
}

CBTextEdit::~CBTextEdit()
{
    qDebug("~CBTextEdit");
    _runThread = false;
    _readTargetThread.join();
}

void CBTextEdit::setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface)
{
    if (_tgtTerminal == NULL)
    {
        _tgtTerminal.reset(new TgtTerminal(targetInterface));
        targetInterface->TgtConnect();
        _readTargetThread = boost::thread(&CBTextEdit::readTarget, this);
    }
    else
    {
        qDebug("setTargetInterface called more than once...");
    }
}

void CBTextEdit::keyPressEvent(QKeyEvent *e)
{
    QChar ch = e->text()[0];

    if (ch.isPrint())
    {
        _tgtTerminal->vt_send(ch.toLatin1());
    }
    else
    {
        switch (e->key())
        {
        case Qt::Key_Tab:
            _tgtTerminal->vt_send('\t');
            break;
        case Qt::Key_Backspace:
            _tgtTerminal->vt_send(KEY_BACKSPACE);
            break;
        case Qt::Key_Escape:
            _tgtTerminal->vt_send('\33');
            break;
        case Qt::Key_Return:
            _tgtTerminal->vt_send('\r');
            break;
        case Qt::Key_Left:
            _tgtTerminal->vt_send(K_LT);
            break;
        case Qt::Key_Right:
            _tgtTerminal->vt_send(K_RT);
            break;
        case Qt::Key_Up:
            _tgtTerminal->vt_send(K_UP);
            break;
        case Qt::Key_Down:
            _tgtTerminal->vt_send(K_DN);
            break;
        case Qt::Key_PageUp:
            _tgtTerminal->vt_send(K_PGUP);
            break;
        case Qt::Key_PageDown:
            _tgtTerminal->vt_send(K_PGDN);
            break;
        case Qt::Key_Home:
            _tgtTerminal->vt_send(K_HOME);
            break;
        case Qt::Key_Insert:
            _tgtTerminal->vt_send(K_INS);
            break;
        case Qt::Key_Delete:
            _tgtTerminal->vt_send(K_DEL);
            break;
        case Qt::Key_End:
            _tgtTerminal->vt_send(K_END);
            break;
        }
    }
}

void CBTextEdit::readTarget()
{
    boost::asio::mutable_buffer b;
    while (_runThread == true)
    {
        int bytes = _tgtTerminal->_targetInterface->TgtRead(b);
        if (bytes > 0)
        {
            const char *data = boost::asio::buffer_cast<const char*>(b);
            qDebug("got: %i", bytes);
            qDebug("[%s]", data);
            for (int i = 0; i < bytes; i++)
            {
                _tgtTerminal->vt_out(data[i]);
            }
            _tgtTerminal->_targetInterface->TgtReturnReadBuffer(b);
            emit textUpdatedSignal();
        }
    }
}

bool CBTextEdit::setCharColor(int *fg, int *bg, const char_t *c)
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
        ret = true;
    }
    *fg = newFg;
    *bg = newBg;
    return ret;
}

QFont CBTextEdit::getFont()
{
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

void CBTextEdit::paintEvent(QPaintEvent *e)
{
    qDebug("PaintEvent");
    QPlainTextEdit::paintEvent(e);
}
#if 0
void CBTextEdit::insertText()
{
    if (_tgtTerminal->getDirty() == true)
    {
        qDebug("insert start");
        clear();
        QTextCursor myCursor = textCursor();
        QTextBlockFormat format;
        format.setBackground(Qt::red);
        myCursor.setBlockFormat(format);

        myCursor.insertText("the ");

        format.setBackground(Qt::green);
        myCursor.insertBlock(format);
        myCursor.insertText("fish ");

        format.setBackground(Qt::yellow);
        myCursor.insertBlock(format);
        myCursor.insertText("are ");

        format.setBackground(Qt::red);
        myCursor.insertBlock(format);
        myCursor.insertText("coming!");

        format.setBackground(Qt::green);
        myCursor.insertBlock(format);
        myCursor.insertText(QString("%1 blocks").arg(blockCount()));
        myCursor.movePosition(QTextCursor::StartOfLine);
        myCursor.select(QTextCursor::LineUnderCursor);
        myCursor.insertText(QString("XXXX"));
        _tgtTerminal->setDirty(false);
        viewport()->update();
        qDebug("insert done");
    }
}
#endif
#if 0
QTextBlock CBTextEdit::addMinBlocks()
{
    QTextCursor myCursor = textCursor();
    QTextBlock block;
    QTextDocument *doc = document();
    int wsrow = _tgtTerminal->getWinSizeRow();
    int bc = doc->blockCount();
    int max = doc->maximumBlockCount();
    while (bc < wsrow)
    {
        myCursor.insertBlock();
        bc = doc->blockCount();
    }
    block = doc->findBlockByNumber(doc->lastBlock().blockNumber() - _tgtTerminal->getWinSizeRow());
    return block;
}
#endif
void CBTextEdit::insertLine(int y, int *fg, int *bg, QTextCursor &cursor)
{
    int x;
    QString text;
    bool draw = false;
    QTextCharFormat format;

    for (x = 0; x < _tgtTerminal->getWinSizeCol(); x++)
    {
        const char_t *c = _tgtTerminal->getChar(x, y);
        const uchar code = c->text;

        if (code != 0)
        {
            draw = setCharColor(fg, bg, c);
            format.setForeground(QBrush(_colors[*fg]));
            format.setBackground(QBrush(_colors[*bg]));
        }
        else
        {
            draw = true;
        }
        if ((text.length() > 0) && (draw == true))
        {
            cursor.insertText(text, format);
            text.clear();
            draw = false;
        }
        if (code != 0)
        {
            text.append(code);
        }
    }
}
#if 1
void CBTextEdit::insertText()
{
#if 1
    if (_tgtTerminal->getDirty() == true)
    {
        int y;
        int fg = -1;
        int bg = -1;
        QTextBlock block = document()->firstBlock();
        for (y = 0; y < _tgtTerminal->getWinSizeRow(); y++)
        {
            QTextCursor cursor(block);
            insertLine(y, &fg, &bg, cursor);
            block = block.next();
            if (block.isValid() == false)
            {
                cursor.insertBlock();
                block = document()->lastBlock();
            }
        }
        _tgtTerminal->setDirty(false);
        viewport()->update();
    }
#else
    int x = 0;
    int y = 0;
    int px = 0;
    int py = 0;
    int fg = -1;
    int bg = -1;
    QString text;
    QPainter painter(this->viewport());
    QFont f = getFont();
    painter.setFont(f);
    QFontMetrics fm(f);
    bool draw = false;
    int textHeight = fm.ascent() + fm.descent();
    QPen curPen = painter.pen();
    const char_t *c = _tgtTerminal->getChar(0, 0);
    setCharColor(&fg, &bg, c);
    curPen.setColor(QColor(_colors[fg]));
    painter.setPen(curPen);

    for (y = 0; y < _tgtTerminal->getWinSizeRow(); y++)
    {
        px = 0;
        py += textHeight;
        draw = false;
        for (x = 0; x < _tgtTerminal->getWinSizeCol(); x++)
        {
            c = _tgtTerminal->getChar(x, y);
            uchar code = c->text;

            if (code != 0)
            {
                draw = setCharColor(&fg, &bg, c);
            }
            else
            {
                draw = true;
            }

            if ((text.length() > 0) && (draw == true))
            {
                painter.drawText(QPoint(px, py), text);
                px += fm.width(text);
                text.clear();
                draw = false;
                curPen.setColor(QColor(_colors[fg]));
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
    _tgtTerminal->setDirty(false);
#endif
}
#endif
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


void CBTextEdit::textUpdatedSlot()
{
    insertText();
}
