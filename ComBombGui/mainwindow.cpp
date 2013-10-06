#include <QMdiSubWindow>
#include <QMessageBox>
#include "mainwindow.h"
#include "childform.h"
#include "aboutdialog.h"
#include "ui_mainwindow.h"

#define CB_FILE_CONFIG_STR  "file"
#define CB_SSH_CONFIG_STR   "ssh"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    QCoreApplication::setOrganizationName("Desjardins");
    QCoreApplication::setOrganizationDomain("chrisd.info");
    QCoreApplication::setApplicationName("ComBomb");

    _ui->setupUi(this);
    _mdiArea = new QMdiArea;
    setCentralWidget(_mdiArea);
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

void MainWindow::getPreviousConnections(const std::string &connType, QStringList* connections)
{
    QSettings settings;
    int size = settings.beginReadArray(connType.c_str());
    int i;
    for (i = 0; i < size; ++i)
    {
        settings.setArrayIndex(i);
        connections->append(settings.value("config").toString());
    }
    settings.endArray();
}

void MainWindow::saveConnections(const std::string &connType, const std::string &connStr)
{
    if (connType.length() > 0)
    {
        int i;
        QStringList connections;
        QSettings settings;
        getPreviousConnections(connType, &connections);
        connections.insert(0, connStr.c_str());
        connections.removeDuplicates();
        i = 0;
        settings.beginWriteArray(connType.c_str());
        for (QStringList::iterator it = connections.begin(); it != connections.end(); ++it)
        {
            settings.setArrayIndex(i++);
            settings.setValue("config", *it);
        }
        settings.endArray();
    }
}

void MainWindow::loadConnections(const std::string &connType, OpenDialog &openDialog)
{
    QStringList connections;
    getPreviousConnections(connType, &connections);

    for (QStringList::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        std::stringstream s;
        s << it->toLocal8Bit().constData();
        boost::archive::text_iarchive ia(s);
        if (connType.compare(CB_FILE_CONFIG_STR) == 0)
        {
            TgtFileIntf::TgtConnectionConfig f;
            ia >> f;
            openDialog.addFileConfig(f);
        }
        else if (connType.compare(CB_SSH_CONFIG_STR) == 0)
        {
            TgtSshIntf::TgtConnectionConfig s;
            ia >> s;
            openDialog.addSshConfig(s);
        }
    }
}

void MainWindow::on_actionOpen_triggered()
{
    std::stringstream ofs;
    boost::archive::text_oarchive oa(ofs);
    OpenDialog openDialog(this);
    std::map<std::string, std::string> connConfig;
    loadConnections(CB_FILE_CONFIG_STR, openDialog);
    loadConnections(CB_SSH_CONFIG_STR, openDialog);

    if (openDialog.exec() == OpenDialog::Accepted)
    {
        try
        {
            boost::shared_ptr<TgtIntf> intf;
            switch (openDialog.getConnectionType())
            {
                case OpenDialog::CB_CONN_SERIAL:
                {
                    boost::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> p = openDialog.getSerialConfig();
                    intf = TgtSerialIntf::createSerialConnection(p);
                }
                break;
                case OpenDialog::CB_CONN_FILE:
                {
                    boost::shared_ptr<const TgtFileIntf::TgtConnectionConfig> p = openDialog.getFileConfig();
                    intf = TgtFileIntf::createFileConnection(p);
                    oa << *(p.get());
                    connConfig[CB_FILE_CONFIG_STR] = ofs.str();
                }
                break;
                case OpenDialog::CB_CONN_SSH:
                {
                    boost::shared_ptr<const TgtSshIntf::TgtConnectionConfig> p = openDialog.getSshConfig();
                    intf = TgtSshIntf::createSshConnection(p);
                    oa << *(p.get());
                    connConfig[CB_SSH_CONFIG_STR] = ofs.str();
                }
                break;
            }

            ChildForm* childForm = new ChildForm(intf);
            connect(intf.get(), SIGNAL(updateStatusSignal(QString)), this, SLOT(updateStatusSlot(QString)));

            QMdiSubWindow* subWindow = _mdiArea->addSubWindow(childForm);
            _connections.push_back(intf);
            if (connConfig.size() > 0)
            {
                saveConnections(connConfig.begin()->first, connConfig.begin()->second);
            }
            subWindow->show();
            _ui->_statusBar->showMessage("Opened connection", 5000);
        }
        catch (const std::exception &e)
        {
            MainWindow::errorBox(e.what());
        }
    }
}

void MainWindow::updateStatusSlot(QString status)
{
    _ui->_statusBar->showMessage(status, 5000);
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionAbout_ComBomb_triggered()
{
    AboutDialog about(this);
    about.exec();
}
