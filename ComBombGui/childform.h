#ifndef CB_CHILDFORM_H
#define CB_CHILDFORM_H

#include <QWidget>
#include <QMutex>
#include "QTerminal/TgtIntf.h"
#include "QTerminal/QTerminal"

namespace Ui {
class ChildForm;
}

class ChildForm : public QTerminal
{
    Q_OBJECT

public:
    explicit ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent = 0);
    void runProcess();
    ~ChildForm();
private slots:
    void updateTitleSlot(QString title);
    void readFromStdout();
    void processError(QProcess::ProcessError error);
    void processDone(int returnCode, QProcess::ExitStatus status);
    void onReceiveText(const QString&);
signals:
    void updateStatusSignal(QString);
protected:
    virtual void closeEvent(QCloseEvent* event);
private:
    void deleteProcess();
    QMutex _mutex;
    Ui::ChildForm* ui;
    QProcess *_proc;

};

#endif // CHILDFORM_H
