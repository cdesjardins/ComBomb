#ifndef CJD_OPENDIALOG_H
#define CJD_OPENDIALOG_H

#include <QDialog>
#include "TargetConnection/TgtConnection.h"

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog
{
    Q_OBJECT

public:
    enum ConnectionType
    {
        CB_CONN_SSH,
        CB_CONN_SERIAL,
        CB_CONN_FILE,
    };

    explicit OpenDialog(QWidget* parent = 0);
    ~OpenDialog();
    boost::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> getSerialConfig() const;
    boost::shared_ptr<const TgtFileIntf::TgtConnectionConfig> getFileConfig() const;
    boost::shared_ptr<const TgtSshIntf::TgtConnectionConfig> getSshConfig() const;

    ConnectionType getConnectionType();

private slots:
    void on__browseButton_clicked();
private:
    void addComPorts();
    void addComPorts(const std::string &basePortName);
    void addBaudRates();
    void addParity();
    void addStopBits();
    void addByteSize();
    void addFlowControl();

    Ui::OpenDialog* ui;
};

#endif // OPENDIALOG_H
