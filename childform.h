#ifndef CHILDFORM_H
#define CHILDFORM_H

#include <QWidget>

namespace Ui {
class ChildForm;
}

class ChildForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChildForm(QWidget *parent = 0);
    ~ChildForm();
    
private:
    Ui::ChildForm *ui;
};

#endif // CHILDFORM_H
