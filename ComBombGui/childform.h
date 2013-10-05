#ifndef CJD_CHILDFORM_H
#define CJD_CHILDFORM_H

#include <QWidget>
#include "TargetIntf.h"
#include "QTerminal/QTerminal"

namespace Ui {
class ChildForm;
}

class ChildForm : public QTerminal
{
    Q_OBJECT
    
public:
    explicit ChildForm(const boost::shared_ptr<TgtIntf> &targetInterface, QWidget *parent = 0);
    ~ChildForm();
    //void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);

private:
    Ui::ChildForm *ui;
};

#endif // CHILDFORM_H
