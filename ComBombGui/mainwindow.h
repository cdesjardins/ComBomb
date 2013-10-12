#ifndef CJD_MAINWINDOW_H
#define CJD_MAINWINDOW_H

#include "opendialog.h"
#include <QMdiArea>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    static void errorBox(QString errMsg);
protected:
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

private:

    Ui::MainWindow* _ui;
    QMdiArea* _mdiArea;
};

#endif // MAINWINDOW_H
