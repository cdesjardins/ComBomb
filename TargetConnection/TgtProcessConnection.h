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
#ifndef CB_TGT_PROCESS_CONNECTION_H
#define CB_TGT_PROCESS_CONNECTION_H

#include "../QTerminal/TgtIntf.h"
#include "TgtThread.h"
#include <QProcess>
#include <QMutex>

class TgtProcessIntf : public TgtIntf
{
    Q_OBJECT
public:
    struct TgtConnectionConfig : public TgtIntf::TgtConnectionConfigBase
    {
        TgtConnectionConfig(std::string program, std::string workingDir, std::string args)
            : _program(program),
            _workingDir(workingDir),
            _args(args)
        {
        }

        std::string _program;
        std::string _workingDir;
        std::string _args;
    };
    static std::shared_ptr<TgtProcessIntf> createProcessConnection(
        const std::shared_ptr<const TgtConnectionConfig>& config);

    TgtProcessIntf(const std::shared_ptr<const TgtConnectionConfig>& config);
    virtual ~TgtProcessIntf();

    virtual void tgtGetTitle(std::string* szTitle);
private slots:
    void readFromStdout();
    void readFromStderr();
    void processError(QProcess::ProcessError error);
    void processDone(int returnCode, QProcess::ExitStatus status);
protected:
    virtual void tgtMakeConnection();
    virtual void tgtBreakConnection();
    virtual void deleteProcess();
    virtual void closeEvent(QCloseEvent* event);
    void processInput(const QByteArray& output);
    bool writerThread();

    QProcess* _proc;
    QMutex _processMutex;
    std::shared_ptr<TgtThread> _processWriterThread;
};

#endif
