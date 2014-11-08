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
#ifndef CB_CHILDFORM_H
#define CB_CHILDFORM_H

#include <QWidget>
#include <QMutex>
#include "QTerminal/TgtIntf.h"
#include "QTerminal/QTerminal"

namespace Ui {
class ChildForm;
}

class ChildForm : public QTerminal
{
    Q_OBJECT

public:
    explicit ChildForm(const QTerminalConfig &terminalConfig, const boost::shared_ptr<TgtIntf> &targetInterface, QWidget* parent = 0);
    void runProcess();
    void findText();
    void findTextNext(const bool backward);
    void findTextHighlighted();
    virtual ~ChildForm();
private slots:
    void updateTitleSlot(QString title);
    void readFromStdout();
    void readFromStderr();
    void processError(QProcess::ProcessError error);
    void processDone(int returnCode, QProcess::ExitStatus status);
    void onReceiveBlock(boost::intrusive_ptr<RefCntBuffer> incoming);
signals:
    void updateStatusSignal(QString);
    void openWindowSignal();
    void closeWindowSignal();
protected:
    virtual void closeEvent(QCloseEvent* event);
private:
    void deleteProcess();
    void readFromProc(bool isStdout);
    QMutex _processMutex;
    Ui::ChildForm* ui;
    QProcess *_proc;
    bool _procError;
    QString _lastSearchString;
    bool _lastSearchCaseSensitivity;
    bool _redirectStdout;
    bool _redirectStderr;
};

#endif // CHILDFORM_H
