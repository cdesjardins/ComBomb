#ifndef CJD_MAINWINDOW_H
#define CJD_MAINWINDOW_H

#include "opendialog.h"
#include <QMainWindow>
#include <QMdiArea>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
    void on_actionOpen_triggered();


private:

    OpenDialog *_openDialog;
    Ui::MainWindow *_ui;
    QMdiArea *_mdiArea;
};

#endif // MAINWINDOW_H
