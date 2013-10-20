#ifndef CJD_CHILDFORM_H
#define CJD_CHILDFORM_H

#include <QWidget>
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
    ~ChildForm();
private slots:
    void updateTitleSlot(QString title);
private:
    Ui::ChildForm* ui;
};

#endif // CHILDFORM_H
