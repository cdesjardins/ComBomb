#include <QMdiSubWindow>
#include "mainwindow.h"
#include "childform.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    _mdiArea = new QMdiArea;
    setCentralWidget(_mdiArea);
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::on_actionOpen_triggered()
{
    QMdiSubWindow *subWindow = _mdiArea->addSubWindow(new ChildForm());
    subWindow->show();
}
