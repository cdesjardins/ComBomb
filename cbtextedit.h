#ifndef CJD_CBTEXTEDIT_H
#define CJD_CBTEXTEDIT_H

#include "tgtterminal.h"
#include <QPlainTextEdit>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#endif

class CBTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);
signals:
    void textUpdatedSignal();
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void readTarget();
    void paintScreen();
    bool setCharColor(int *fg, int *bg, const char_t *c);
    void paintEvent(QPaintEvent *e);
    void mypaint();
    QFont getFont();
    QSize sizeHint() const;
    void insertLine(int y, int *fg, int *bg, QTextCursor &cursor);

    volatile bool _runThread;
    boost::thread _readTargetThread;
    std::vector<QColor> _colors;
    boost::scoped_ptr<TgtTerminal> _tgtTerminal;

private slots:
    void insertText();
    void textUpdatedSlot();
};

#endif // CBTEXTEDIT_H
