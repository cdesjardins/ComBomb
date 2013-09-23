#ifndef CJD_CBTEXTEDIT_H
#define CJD_CBTEXTEDIT_H

#include "cbtextdocument.h"
#include "TargetIntf.h"
#include <QPlainTextEdit>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#endif

class CBTextEdit : public QPlainTextEdit
{
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void readTarget();
    void paintScreen();
    bool setCharColor(int *fg, int *bg, const char_t *c);
    void paintEvent(QPaintEvent *e);
    void mypaint();
    void insertText();
    QFont getFont();
    QSize sizeHint() const;

    void drawLineCharString(QPainter& painter, int x, int y, const QString& str);

    volatile bool _runThread;
    boost::thread _readTargetThread;
    //boost::thread _paintScreenThread;
    std::vector<QColor> _colors;
    boost::scoped_ptr<CBTextDocument> _cbTextDocument;
};

#endif // CBTEXTEDIT_H
