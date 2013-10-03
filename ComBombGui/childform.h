#ifndef CJD_CHILDFORM_H
#define CJD_CHILDFORM_H

#include <QWidget>
#include "TargetIntf.h"
#include "QTerminal"

namespace Ui {
class ChildForm;
}

class ChildForm : public QTerminal
{
    Q_OBJECT
    
public:
    explicit ChildForm(QWidget *parent = 0);
    ~ChildForm();
    void setTargetInterface(const boost::shared_ptr<TgtIntf> &targetInterface);

private:
    Ui::ChildForm *ui;
};

#endif // CHILDFORM_H
