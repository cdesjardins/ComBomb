#ifndef CBTEXTEDIT_H
#define CBTEXTEDIT_H

#include "TargetIntf.h"
#include <QTextEdit>
#include <Boost/thread.hpp>

class CBTextEdit : public QTextEdit
{
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(TgtIntf* targetInterface);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    TgtIntf *_targetInterface;
    void readTarget();
    volatile bool _runThread;
    boost::thread _readTargetThread;
};

#endif // CBTEXTEDIT_H
