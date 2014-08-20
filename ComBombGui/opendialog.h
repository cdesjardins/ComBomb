#ifndef CB_OPENDIALOG_H
#define CB_OPENDIALOG_H

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
        CB_CONN_FILE
    };

    explicit OpenDialog(QWidget* parent = 0);
    virtual ~OpenDialog();
    boost::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> getSerialConfig() const;
    boost::shared_ptr<const TgtFileIntf::TgtConnectionConfig> getFileConfig() const;
    boost::shared_ptr<const TgtSshIntf::TgtConnectionConfig> getSshConfig() const;
    void addFileConfig(const TgtFileIntf::TgtConnectionConfig &config);
    void addSshConfig(const TgtSshIntf::TgtConnectionConfig &config);

    ConnectionType getConnectionType();

private slots:
    void on_browseButton_clicked();
    void hostNameSelectionChanged(int x);
    void on_privKeyBrowseButton_clicked();
    void on__buttonBox_accepted();

private:
    void addComPorts();
    void addBaudRates();
    void addParity();
    void addStopBits();
    void addByteSize();
    void addFlowControl();

    Ui::OpenDialog* ui;
};

#endif // OPENDIALOG_H
