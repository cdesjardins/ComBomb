#ifndef CJD_CBTEXTEDIT_H
#define CJD_CBTEXTEDIT_H

#include "tgtterminal.h"
#include <QPlainTextEdit>
#ifndef Q_MOC_RUN
#include <boost/thread.hpp>
#endif
#define OUT_TO_DEBUG_FILE
class CBTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    CBTextEdit(QWidget *parent = 0);
    ~CBTextEdit();
    void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);
signals:
    void textUpdatedSignal(int scrollCnt);
protected:
    virtual void keyPressEvent(QKeyEvent *e);
    void readTarget();
    void paintScreen();
    bool getCharColors(int *fg, int *bg, const char_t &c);
    void setFormatColors(QTextCharFormat &format, int fg, int bg);
    void paintEvent(QPaintEvent *e);
    void mypaint();
    QFont getFont();
    QSize sizeHint() const;
    void insertLine(int y, int *fg, int *bg, QTextCursor &cursor);

    volatile bool _runThread;
    boost::thread _readTargetThread;
    std::vector<QColor> _colors;
    boost::scoped_ptr<TgtTerminal> _tgtTerminal;
#ifdef OUT_TO_DEBUG_FILE
    std::string _debugFileName;
    std::ofstream _debugFile;
#endif
private slots:
    void insertText(int scrollCnt);
    void textUpdatedSlot(int scrollCnt);
};

#endif // CBTEXTEDIT_H
