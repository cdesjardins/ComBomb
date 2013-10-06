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
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    static void errorBox(QString errMsg);
private slots:
    void on_actionOpen_triggered();
    void updateStatusSlot(QString status);
private:

    Ui::MainWindow* _ui;
    QMdiArea* _mdiArea;
    std::vector<boost::shared_ptr<TgtIntf> > _connections;
};

#endif // MAINWINDOW_H
