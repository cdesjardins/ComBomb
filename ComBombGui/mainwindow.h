#ifndef CB_MAINWINDOW_H
#define CB_MAINWINDOW_H

#include "fileclipboarddialog.h"
#include "opendialog.h"
#include "childform.h"
#include <QMdiArea>
#include <QMainWindow>
#include <boost/atomic.hpp>

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
protected:
    explicit MainWindow(QWidget* parent = 0);
    void enableMenuItems(bool enabled);
    void closeEvent(QCloseEvent *event);
    void readSettings();
private slots:
    void openWindowSlot();
    void closeWindowSlot();
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

signals:
    void findSignal();
private:

    Ui::MainWindow* _ui;
    QMdiArea* _mdiArea;
    FileClipboardDialog* _fileClipboardDialog;
    static MainWindow* _instance;
    boost::atomic<int> _windowCnt;
    QString _runProcessIconText;
};

#endif // MAINWINDOW_H
