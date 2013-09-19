#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QDialog>
#include "TargetIntf.h"

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit OpenDialog(QWidget *parent = 0);
    ~OpenDialog();
    const TgtSerialIntf::TgtConnection getSerialConfig() const;
private:
    void detectComPorts();
    std::vector<std::string> _comPorts;

    Ui::OpenDialog *ui;
};

#endif // OPENDIALOG_H
