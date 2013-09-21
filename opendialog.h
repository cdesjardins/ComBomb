#ifndef CJD_OPENDIALOG_H
#define CJD_OPENDIALOG_H

#include <QDialog>
#include "TargetIntf.h"

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog
{
    Q_OBJECT
    
public:
    enum ConnectionType
    {
        CB_CONN_SERIAL,
        CB_CONN_FILE
    };

    explicit OpenDialog(QWidget *parent = 0);
    ~OpenDialog();
    const TgtSerialIntf::TgtConnection getSerialConfig() const;
    const TgtFileIntf::TgtConnection getFileConfig() const;

    ConnectionType getConnectionType();
private slots:
    void on__browseButton_clicked();

private:
    void addComPorts();
    void addBaudRates();
    void addParity();
    void addStopBits();
    void addByteSize();
    void addFlowControl();

    Ui::OpenDialog *ui;
};

#endif // OPENDIALOG_H
