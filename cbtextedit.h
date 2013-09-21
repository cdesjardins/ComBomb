#ifndef CJD_CBTEXTEDIT_H
#define CJD_CBTEXTEDIT_H

#include "TargetIntf.h"
#include "vt100.h"
#include <QTextEdit>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#endif

class CBTextEdit : public QTextEdit, Terminal
{
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);
    virtual void char_out(char c);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void readTarget();
    void paintScreen();
    bool setCharColor(int *fg, int *bg, char_t *c, QPen *newPen);
    void paintEvent(QPaintEvent *e);
    void mypaint();
    QSize sizeHint() const;

    void CBTextEdit::drawLineCharString(QPainter& painter, int x, int y, const QString& str);
    boost::shared_ptr<TgtIntf> _targetInterface;

    volatile bool _runThread;
    boost::thread _readTargetThread;
    //boost::thread _paintScreenThread;
    std::vector<QColor> _colors;
};

#endif // CBTEXTEDIT_H
