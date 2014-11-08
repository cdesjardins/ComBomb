/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
        CB_CONN_PROCESS,
    };

    explicit OpenDialog(QWidget* parent = 0);
    virtual ~OpenDialog();

    boost::shared_ptr<const TgtSshIntf::TgtConnectionConfig> getSshConfig() const;
    boost::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> getSerialConfig() const;
    boost::shared_ptr<const TgtProcessIntf::TgtConnectionConfig> getProcessConfig() const;
    void addSshConfig(const TgtSshIntf::TgtConnectionConfig &config);

    ConnectionType getConnectionType();

    bool newlines();

private slots:
    void hostNameSelectionChanged(int x);
    void on_privKeyBrowseButton_clicked();
    void on__buttonBox_accepted();

    void on_programBrowseButton_clicked();

    void on_workingDirButton_clicked();

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
