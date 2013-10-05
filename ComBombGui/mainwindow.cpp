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
        MainWindow::errorBox("Unable to init crypt lib");
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

void MainWindow::errorBox(QString errMsg)
{
    QMessageBox msgBox;
    msgBox.setText(errMsg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setWindowIcon(QIcon(":/images/ComBomb16.png"));
    msgBox.setWindowTitle("Error");
    msgBox.exec();
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
            _ui->_statusBar->showMessage("Opened connection", 5000);
        }
        catch (const std::exception &e)
        {
            MainWindow::errorBox(e.what());
        }
    }
}

