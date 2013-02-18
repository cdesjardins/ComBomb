#ifndef CBTEXTEDIT_H
#define CBTEXTEDIT_H

#include "TargetIntf.h"
#include "vt100.h"
#include <QTextEdit>
#include <Boost/thread.hpp>

class CBTextEdit : public QTextEdit, Terminal
{
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(TgtIntf* targetInterface);
    virtual void char_out(char c);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    TgtIntf *_targetInterface;
    void readTarget();
    volatile bool _runThread;
    boost::thread _readTargetThread;
};

#endif // CBTEXTEDIT_H
