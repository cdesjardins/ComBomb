#include <QMdiSubWindow>
#include <QMessageBox>
#include "mainwindow.h"
#include "childform.h"
#include "TargetIntf.h"
#include "ui_mainwindow.h"
#include "cryptlib.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);
    _mdiArea = new QMdiArea;
    setCentralWidget(_mdiArea);
    _openDialog = new OpenDialog();

    int status = cryptInit();
    if (cryptStatusError(status))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to init crypt lib");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
}

MainWindow::~MainWindow()
{
    std::vector<boost::shared_ptr<TgtIntf> >::iterator it;
    for (it = _connections.begin(); it < _connections.end(); it++)
    {
        boost::shared_ptr<TgtIntf> connection = *it;
        connection->TgtDisconnect();
    }
    _connections.clear();
    delete _ui;
    cryptEnd();
    qDebug("~mainwindow");
}

void MainWindow::on_actionOpen_triggered()
{
    if (_openDialog->exec() == OpenDialog::Accepted)
    {
        try
        {
            boost::shared_ptr<TgtIntf> intf;
            switch (_openDialog->getConnectionType())
            {
                case OpenDialog::CB_CONN_SERIAL:
                    intf = TgtSerialIntf::createSerialConnection(_openDialog->getSerialConfig());
                    break;
                case OpenDialog::CB_CONN_FILE:
                    intf = TgtFileIntf::createFileConnection(_openDialog->getFileConfig());
                    break;
                case OpenDialog::CB_CONN_SSH:
                    intf = TgtSshIntf::createSshConnection(_openDialog->getSshConfig());
                    break;
            }

            ChildForm* childForm = new ChildForm(intf);
            QMdiSubWindow* subWindow = _mdiArea->addSubWindow(childForm);
            _connections.push_back(intf);
            subWindow->show();
        }
        catch (const std::exception &e)
        {
            QMessageBox messageBox;
            QString err = "Unable to open connection to: ";
            err.append(e.what());
            messageBox.critical(0, "Error", err);
            messageBox.setFixedSize(500, 200);
        }
    }
}

