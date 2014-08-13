#include <QMdiSubWindow>
#include <QMessageBox>
#include <QComboBox>
#include "mainwindow.h"
#include "childform.h"
#include "aboutdialog.h"
#include "configdialog.h"
#include "ui_mainwindow.h"

MainWindow* MainWindow ::_instance = NULL;

MainWindow* MainWindow::getMainWindow(QWidget* parent)
{
    if (_instance == NULL)
    {
        _instance = new MainWindow(parent);
    }
    return _instance;
}

void MainWindow::destroyMainWindow()
{
    if (_instance != NULL)
    {
        delete _instance;
        _instance = NULL;
    }
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _mdiArea(new QMdiArea()),
    _fileClipboardDialog(new FileClipboardDialog(this)),
    _windowCnt(0)
{
    _ui->setupUi(this);
    setCentralWidget(_mdiArea);
    enableMenuItems(false);
    readSettings();
}

void MainWindow::enableMenuItems(bool enabled)
{
    _ui->action_Run_Process->setEnabled(enabled);
    _ui->actionCopy->setEnabled(enabled);
    _ui->actionPaste->setEnabled(enabled);
    _ui->actionSelect_All->setEnabled(enabled);
    _ui->action_Clear_scrollback->setEnabled(enabled);
}

MainWindow::~MainWindow()
{
    delete _fileClipboardDialog;
    delete _mdiArea;
    delete _ui;
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
    OpenDialog openDialog(this);

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
                }
                break;
                case OpenDialog::CB_CONN_SSH:
                {
                    boost::shared_ptr<const TgtSshIntf::TgtConnectionConfig> p = openDialog.getSshConfig();
                    intf = TgtSshIntf::createSshConnection(p);
                }
                break;
            }
            QTerminalConfig terminalConfig;
            ConfigDialog::getTerminalConfig(&terminalConfig);
            ChildForm* childForm = new ChildForm(terminalConfig, intf, this);
            connect(intf.get(), SIGNAL(updateStatusSignal(QString)), this, SLOT(updateStatusSlot(QString)));

            QMdiSubWindow* subWindow = _mdiArea->addSubWindow(childForm);
            subWindow->show();
            _ui->statusBar->showMessage("Opened connection", 5000);
        }
        catch (const std::exception &e)
        {
            MainWindow::errorBox(e.what());
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
}

void MainWindow::updateStatusSlot(QString status)
{
    _ui->statusBar->showMessage(status, 5000);
}

void MainWindow::openWindowSlot()
{
    _windowCnt.fetch_add(1);
    enableMenuItems(true);
}

void MainWindow::closeWindowSlot()
{
    int cnt = _windowCnt.fetch_sub(1);
    if (cnt == 1)
    {
        enableMenuItems(false);
    }
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

void MainWindow::on_actionCopy_triggered()
{
    ChildForm* childForm = getActiveChildWindow();
    if (childForm != NULL)
    {
        emit childForm->triggerCopy();
    }
}

void MainWindow::on_actionPaste_triggered()
{
    ChildForm* childForm = getActiveChildWindow();
    if (childForm != NULL)
    {
        emit childForm->triggerPaste();
    }
}

ChildForm* MainWindow::getActiveChildWindow()
{
    QMdiSubWindow* subWindow = _mdiArea->activeSubWindow();
    if (subWindow != NULL)
    {
        ChildForm* childForm = dynamic_cast<ChildForm*>(subWindow->widget());
        return childForm;
    }
    return NULL;
}

void MainWindow::on_actionFile_clipboard_triggered()
{
    if (_fileClipboardDialog->isVisible() == true)
    {
        _fileClipboardDialog->hide();
    }
    else
    {
        _fileClipboardDialog->show();
    }
}

void MainWindow::on_action_Options_triggered()
{
    ConfigDialog configDialog(this);
    configDialog.exec();
}

void MainWindow::on_action_Run_Process_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != NULL)
    {
        activeWindow->runProcess();
    }
}

void MainWindow::on_action_Clear_scrollback_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != NULL)
    {
        activeWindow->clearScrollback();
    }
}

