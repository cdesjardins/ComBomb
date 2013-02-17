#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QDialog>

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OpenDialog(QWidget *parent = 0);
    ~OpenDialog();
    
private:
    void detectComPorts();
    std::vector<std::string> _comPorts;

    Ui::OpenDialog *ui;
};

#endif // OPENDIALOG_H
