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
#ifndef CB_MAINWINDOW_H
#define CB_MAINWINDOW_H

#include "fileclipboarddialog.h"
#include "opendialog.h"
#include "childform.h"
#include <QMdiArea>
#include <QMainWindow>
#include <atomic>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* getMainWindow(QWidget* parent = 0);
    static void destroyMainWindow();
    virtual ~MainWindow();
    static void errorBox(QString errMsg);
    ChildForm* getActiveChildWindow();
    void swapProcessIcon(bool processRunning);
    void swapCaptureIcon(bool captureRunning);
    const std::chrono::duration<double> getStartTimeDelta();
    void setInterfaceType();
protected:
    explicit MainWindow(QWidget* parent = 0);
    void enableMenuItems(bool enabled);
    void closeEvent(QCloseEvent* event);
    void readSettings();
private slots:
    void openWindowSlot();
    void closeWindowSlot();
    void subWindowActivatedSlot(QMdiSubWindow* subWindow);
    void updateStatusSlot(QString status);
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_ComBomb_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionFile_clipboard_triggered();
    void on_action_Options_triggered();
    void on_action_Run_Process_triggered();
    void on_action_Clear_scrollback_triggered();
    void on_actionSelect_All_triggered();
    void on_action_Find_triggered();
    void on_actionFind_next_triggered();
    void on_actionFind_prev_triggered();
    void on_actionFind_highlighted_text_triggered();
    void newVersionAvailableSlot();
    void on_actionNew_Version_Available_triggered();

    void on_actionCapture_output_triggered();

signals:
    void findSignal();
private:

    Ui::MainWindow* _ui;
    QMdiArea* _mdiArea;
    FileClipboardDialog* _fileClipboardDialog;
    static MainWindow* _instance;
    std::atomic_int _windowCnt;
    QString _runProcessIconText;
    QString _captureLogsIconText;
    std::chrono::time_point<std::chrono::system_clock> _startTime;
};

#endif // MAINWINDOW_H
