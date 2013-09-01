#ifndef CBTEXTEDIT_H
#define CBTEXTEDIT_H

#include "TargetIntf.h"
#include "vt100.h"
#include <QTextEdit>
#include <boost/thread.hpp>

class CBTextEdit : public QTextEdit, Terminal
{
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(TgtIntf* targetInterface);
    virtual void char_out(char c);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void readTarget();
    void paintScreen();
    void paintChar(int x, int y, char_t *c);
    void paintEvent(QPaintEvent *e);

    void CBTextEdit::drawLineCharString(QPainter& painter, int x, int y, const QString& str);
    TgtIntf *_targetInterface;

    volatile bool _runThread;
    boost::thread _readTargetThread;
    //boost::thread _paintScreenThread;
    std::vector<QColor> _colors;
};

#endif // CBTEXTEDIT_H
