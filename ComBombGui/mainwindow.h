#ifndef CB_MAINWINDOW_H
#define CB_MAINWINDOW_H

#include "fileclipboarddialog.h"
#include "opendialog.h"
#include "childform.h"
#include <QMdiArea>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* getMainWindow(QWidget* parent = 0);
    static void destroyMainWindow();
    ~MainWindow();
    static void errorBox(QString errMsg);
    static void restoreWidgetGeometry(QWidget* w, QString tag);
    static void saveWidgetGeometry(QWidget* w, QString tag);
    ChildForm* getActiveChildWindow();
protected:
    explicit MainWindow(QWidget* parent = 0);
    void saveConnections(const std::string &connType, const std::string &connStr);
    void loadConnections(const std::string &connType, OpenDialog &openDialog);
    void getPreviousConnections(const std::string &connType, QStringList* connections);
private slots:
    void on_actionOpen_triggered();
    void updateStatusSlot(QString status);
    void on_actionExit_triggered();
    void on_actionAbout_ComBomb_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionFile_clipboard_triggered();
    void on_action_Options_triggered();
    void on_action_Run_Process_triggered();

private:

    Ui::MainWindow* _ui;
    QMdiArea* _mdiArea;
    FileClipboardDialog* _fileClipboardDialog;
    static MainWindow* _instance;
};

#endif // MAINWINDOW_H
