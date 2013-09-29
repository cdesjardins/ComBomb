#include "cbtextedit.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QtGui/QPainter>
#include <boost/date_time/posix_time/posix_time.hpp>

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

    0x555555,
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
#ifdef OUT_TO_DEBUG_FILE
    _debugFileName = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
    _debugFile.open(_debugFileName + ".cbd", std::ios::out | std::ios::binary);
#endif
    for (size_t i = 0; i < (sizeof(_colormap) / sizeof(_colormap[0])); i++)
    {
        _colors.push_back(QColor(QRgb(_colormap[i])));
    }
    textCursor().insertBlock();
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_DeleteOnClose);
}

CBTextEdit::~CBTextEdit()
{
    qDebug("~CBTextEdit");
    _runThread = false;
    _readTargetThread.join();
    _tgtTerminal->_targetInterface->TgtDisconnect();
#ifdef OUT_TO_DEBUG_FILE
    _debugFile.close();
#endif
}

void CBTextEdit::setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface)
{
    if (_tgtTerminal == NULL)
    {
        std::string szTitle;
        _tgtTerminal.reset(new TgtTerminal(targetInterface, 80, 25));
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
    else if (e->matches(QKeySequence::Copy))
    {
        _tgtTerminal->vt_send(K_CTRLC);
    }
    else if (e->matches(QKeySequence::Undo))
    {
        _tgtTerminal->vt_send(K_CTRLZ);
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
        case Qt::Key_Enter:
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
            //qDebug("got: %i", bytes);
            //qDebug("[%s]", data);
            for (int i = 0; i < bytes; i++)
            {
                _tgtTerminal->vt_out(data[i]);
            }
            _tgtTerminal->_targetInterface->TgtReturnReadBuffer(b);
#ifdef OUT_TO_DEBUG_FILE
            _debugFile.write(data, bytes);
#endif
        }
        if ( (_tgtTerminal->isWinDirty() == true))
        {
            emit textUpdatedSignal(0);
        }
    }
}

bool CBTextEdit::getCharColors(int *fg, int *bg, const char_t &c)
{
    bool ret = false;
    int newFg;
    int newBg;

    newBg = COLBG(c.col);
    newFg = COLFG(c.col);
    if (c.attrib & XA_BOLD)
    {
        newFg += 8;
    }
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
    QPlainTextEdit::paintEvent(e);
}

void CBTextEdit::setFormatColors(QTextCharFormat &format, int fg, int bg)
{
    if (fg != -1)
    {
        format.setForeground(QBrush(_colors[fg]));
    }
    if (bg != -1)
    {
        format.setBackground(QBrush(_colors[bg]));
    }
}

void CBTextEdit::insertLine(int y, int *fg, int *bg, QTextCursor &cursor)
{
    int x;
    QString text;
    bool draw = false;
    QTextCharFormat format;
    char_t c = _tgtTerminal->getChar(0, 0);
    int insertCnt = 0;

    cursor.select(QTextCursor::LineUnderCursor);
    cursor.removeSelectedText();

    getCharColors(fg, bg, c);
    setFormatColors(format, *fg, *bg);

    for (x = 0; x <= _tgtTerminal->getWinSizeCol(); x++)
    {
        if (x < _tgtTerminal->getWinSizeCol())
        {
            c = _tgtTerminal->getChar(x, y);
        }

        const uchar code = c.text;
        if (code != 0)
        {
            draw = getCharColors(fg, bg, c);
        }
        else
        {
            draw = true;
        }
        if ((draw == true) || (x == _tgtTerminal->getWinSizeCol()))
        {
            int textLen = text.length();
            if (textLen > 0)
            {
                if (insertCnt < (x - 1))
                {
                    QString padding(x - 1 - textLen - insertCnt, QChar(' '));
                    cursor.insertText(padding, format);
                    insertCnt += padding.length();
                }
                cursor.insertText(text, format);
                text.clear();
                insertCnt += textLen;
            }
            draw = false;
            setFormatColors(format, *fg, *bg);
        }
        if (code != 0)
        {
            text.append(code);
        }
    }
}

void CBTextEdit::insertText(int scrollCnt)
{
    int y;
    int fg = -1;
    int bg = -1;
    QTextBlock block;
    int bc = document()->blockCount();

    block = document()->lastBlock();
    for (y = 0; y < scrollCnt; y++)
    {
        QTextCursor cursor(block);
        cursor.insertBlock();
    }

    if (bc < _tgtTerminal->getWinSizeRow())
    {
        block = document()->firstBlock();
    }
    else
    {
        block = document()->findBlockByNumber(bc - (_tgtTerminal->getWinSizeRow() + 1));
    }

    for (y = 0; y < _tgtTerminal->getWinSizeRow(); y++)
    {
        QTextCursor cursor(block);
        if (_tgtTerminal->isRowDirty(y))
        {
            insertLine(y, &fg, &bg, cursor);
        }
        block = block.next();
        if (block.isValid() == false)
        {
            cursor.insertBlock();
            block = document()->lastBlock();
            qDebug("Adding block: %i", document()->blockCount());
        }
    }

    viewport()->update();
}

QSize CBTextEdit::sizeHint() const
{
    int width = (_tgtTerminal->getWinSizeCol() + 1) * fontMetrics().width('W');
    int height = (_tgtTerminal->getWinSizeRow() + 1) * fontMetrics().lineSpacing();
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

void CBTextEdit::textUpdatedSlot(int scrollCnt)
{
    insertText(scrollCnt);
}
