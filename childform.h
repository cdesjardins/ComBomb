#ifndef CHILDFORM_H
#define CHILDFORM_H

#include <QWidget>
#include "TargetIntf.h"

namespace Ui {
class ChildForm;
}

class ChildForm : public QWidget
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
