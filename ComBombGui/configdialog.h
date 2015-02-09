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
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "cbdialog.h"
#include "QTerminal/QTerminalConfig.h"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget* parent = 0);
    virtual ~ConfigDialog();

    static bool getTerminalConfig(QTerminalConfig* terminalConfig);
    static QStringList getPortListSettings();
    static bool getTabbedViewSettings();
    static bool getBlackBackSettings();
protected:
    void setupAcceptableFontSizes();
    static QStringList getPortListDefaults(QString basePortName, int start, int stop);
    void setPortListSettings();
    void populateComPortListWidget();
    virtual QString getSettingsRoot();

private slots:
    void on_buttonBox_accepted();

    void on_fontComboBox_currentIndexChanged(int index);

private:
    Ui::ConfigDialog* ui;
};

#endif // CONFIGDIALOG_H
