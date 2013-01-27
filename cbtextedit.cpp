#include "cbtextedit.h"

CBTextEdit::CBTextEdit(QWidget *parent):QTextEdit(parent)
{
}


void CBTextEdit::keyPressEvent(QKeyEvent *e)
{
    QTextEdit::keyPressEvent(e);
}
