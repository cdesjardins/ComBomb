#include <QMdiSubWindow>
#include <QMessageBox>
#include "mainwindow.h"
#include "childform.h"
#include "TargetIntf.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    _mdiArea = new QMdiArea;
    setCentralWidget(_mdiArea);
    _openDialog = new OpenDialog();
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::on_actionOpen_triggered()
{
    if (_openDialog->exec() == OpenDialog::Accepted)
    {
        try
        {
            ChildForm *childForm = new ChildForm();
            TgtSerialIntf::TgtConnection config(_openDialog->getSerialConfig());
            boost::shared_ptr<TgtSerialIntf> intf = TgtSerialIntf::createSerialConnection(config);
            childForm->setTargetInterface(intf);

            QMdiSubWindow *subWindow = _mdiArea->addSubWindow(childForm);
            subWindow->show();
        }
        catch (const std::exception &e)
        {
            QMessageBox messageBox;
            QString err = "Unable to open connection to: ";
            err.append(e.what());
            messageBox.critical(0, "Error", err);
            messageBox.setFixedSize(500,200);
        }
    }
}



