#ifndef CBTEXTEDIT_H
#define CBTEXTEDIT_H

#include <QTextEdit>

class CBTextEdit : public QTextEdit
{
public:
    CBTextEdit(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

};

#endif // CBTEXTEDIT_H
