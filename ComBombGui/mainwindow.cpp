/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    https://github.com/cdesjardins/ComBomb cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QComboBox>
#include <QDockWidget>
#include <QToolButton>
#include <QHBoxLayout>
#include <QStyle>
#include <QTabBar>
#include <QPixmap>
#include <QPainter>
#include <QPen>
#include "mainwindow.h"
#include "childform.h"
#include "aboutdialog.h"
#include "configdialog.h"
#include "ui_mainwindow.h"
#include "cppssh/cppssh.h"

MainWindow* MainWindow ::_instance = nullptr;

MainWindow* MainWindow::getMainWindow(QWidget* parent)
{
    if (_instance == nullptr)
    {
        _instance = new MainWindow(parent);
        Cppssh::create();
    }

    return _instance;
}

void MainWindow::destroyMainWindow()
{
    if (_instance != nullptr)
    {
        delete _instance;
        _instance = nullptr;
    }
    Cppssh::destroy();
}

const std::chrono::duration<double> MainWindow::getStartTimeDelta()
{
    return std::chrono::system_clock::now() - _startTime;
}

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _mdiArea(new QMdiArea()),
    _fileClipboardDialog(new FileClipboardDialog(this)),
    _fileClipboarDock(new QDockWidget(tr("File clipboard"), this))
{
    ConfigDialog::handleLogfile();

    _windowCnt.store(0);
    _startTime = std::chrono::system_clock::now();
    _ui->setupUi(this);
    _ui->menuBar->show();
    setMinimumSize(650, 500);

    _mdiArea->setTabsMovable(true);
    _mdiArea->setTabShape(QTabWidget::Triangular);
    _mdiArea->setTabsClosable(false);
    _mdiArea->setViewMode(QMdiArea::SubWindowView);
    setCentralWidget(_mdiArea);
    enableMenuItems(false);
    readSettings();
    _runProcessIconText = _ui->action_Run_Process->text();
    _captureLogsIconText = _ui->actionCapture_output->text();
    connect(_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowActivatedSlot(QMdiSubWindow*)));
}

void MainWindow::enableMenuItems(bool enabled)
{
    _ui->action_Run_Process->setEnabled(enabled);
    _ui->actionCopy->setEnabled(enabled);
    _ui->actionPaste->setEnabled(enabled);
    _ui->actionSelect_All->setEnabled(enabled);
    _ui->action_Clear_scrollback->setEnabled(enabled);
    _ui->action_Find->setEnabled(enabled);
    _ui->actionFind_next->setEnabled(enabled);
    _ui->actionFind_prev->setEnabled(enabled);
    _ui->actionFind_highlighted_text->setEnabled(enabled);
    _ui->actionCapture_output->setEnabled(enabled);
}

void MainWindow::swapProcessIcon(bool processRunning)
{
    if (processRunning == true)
    {
        _ui->action_Run_Process->setIcon(QIcon(":/images/script_delete.png"));
        _ui->action_Run_Process->setText("&Stop process");
    }
    else
    {
        _ui->action_Run_Process->setIcon(QIcon(":/images/script_gear.png"));
        _ui->action_Run_Process->setText(_runProcessIconText);
    }
}

void MainWindow::swapCaptureIcon(bool captureRunning)
{
    if (captureRunning == true)
    {
        _ui->actionCapture_output->setIcon(QIcon(":/images/raw_access_logs_stop.png"));
        _ui->actionCapture_output->setText("&Stop capture");
    }
    else
    {
        _ui->actionCapture_output->setIcon(QIcon(":/images/raw_access_logs.png"));
        _ui->actionCapture_output->setText(_captureLogsIconText);
    }
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
            std::shared_ptr<TgtIntf> intf;
            switch (openDialog.getConnectionType())
            {
                case OpenDialog::CB_CONN_SSH:
                {
                    std::shared_ptr<const TgtCppsshIntf::TgtConnectionConfig> p = openDialog.getSshConfig();
                    intf = TgtCppsshIntf::createCppsshConnection(p);
                }
                break;

                case OpenDialog::CB_CONN_TELNET:
                {
                    std::shared_ptr<const TgtTelnetIntf::TgtConnectionConfig> p = openDialog.getTelnetConfig();
                    intf = TgtTelnetIntf::createTelnetConnection(p);
                }
                break;

                case OpenDialog::CB_CONN_SERIAL:
                {
                    std::shared_ptr<const TgtSerialIntf::TgtConnectionConfig> p = openDialog.getSerialConfig();
                    intf = TgtSerialIntf::createSerialConnection(p);
                }
                break;

                case OpenDialog::CB_CONN_PROCESS:
                {
                    std::shared_ptr<const TgtProcessIntf::TgtConnectionConfig> p = openDialog.getProcessConfig();
                    intf = TgtProcessIntf::createProcessConnection(p);
                }
                break;
            }
            QTerminalConfig terminalConfig;
            ConfigDialog::getTerminalConfig(&terminalConfig);
            ChildForm* childForm = new ChildForm(terminalConfig, intf, this);
            connect(intf.get(), SIGNAL(updateStatusSignal(QString)), this, SLOT(updateStatusSlot(QString)));
            if (openDialog.newlines() == true)
            {
                childForm->newlineToggle();
            }
            QMdiSubWindow* subWindow = _mdiArea->addSubWindow(childForm);
            connect(subWindow, SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)), this, SLOT(subWindowStateChangedSlot(Qt::WindowStates, Qt::WindowStates)));
            subWindow->show();
            _ui->statusBar->showMessage("Opened connection", 5000);
        }
        catch (const std::exception& e)
        {
            MainWindow::errorBox(e.what());
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("WindowState", saveState());
    saveFileClipboarDockSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
    restoreFileClipboardDockSettings();
}

#define FILE_CBD_SETTINGS               "FileClipBoardDock"
#define FILE_CBD_SETTINGS_DOCKSIDE      FILE_CBD_SETTINGS "/DockSide"
#define FILE_CBD_SETTINGS_HIDDEN        FILE_CBD_SETTINGS "/Hidden"
#define FILE_CBD_SETTINGS_FLOATING      FILE_CBD_SETTINGS "/Floating"
#define FILE_CBD_SETTINGS_GEOMETRY      FILE_CBD_SETTINGS "/Geometry"

void MainWindow::saveFileClipboarDockSettings()
{
    QSettings settings;
    settings.setValue(FILE_CBD_SETTINGS_DOCKSIDE, dockWidgetArea(_fileClipboarDock));
    settings.setValue(FILE_CBD_SETTINGS_HIDDEN, _fileClipboarDock->isHidden());
    settings.setValue(FILE_CBD_SETTINGS_FLOATING, _fileClipboarDock->isFloating());
    settings.setValue(FILE_CBD_SETTINGS_GEOMETRY, _fileClipboarDock->saveGeometry());
}

void MainWindow::restoreFileClipboardDockSettings()
{
    QSettings settings;
    _fileClipboarDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _fileClipboarDock->setWidget(_fileClipboardDialog);
    addDockWidget((Qt::DockWidgetArea)settings.value(FILE_CBD_SETTINGS_DOCKSIDE,
                                                     Qt::RightDockWidgetArea).toInt(), _fileClipboarDock);

    _fileClipboarDock->setHidden(settings.value("FileClipBoardDock/Hidden").toBool());
    _fileClipboarDock->setFloating(settings.value("FileClipBoardDock/Floating").toBool());
    _fileClipboarDock->restoreGeometry(settings.value("FileClipBoardDock/Geometry").toByteArray());
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

void MainWindow::subWindowActivatedSlot(QMdiSubWindow* subWindow)
{
    if (subWindow != nullptr)
    {
        ChildForm* childForm = dynamic_cast<ChildForm*>(subWindow->widget());
        swapProcessIcon(childForm->isProcessRunning());
        swapCaptureIcon(childForm->isCaptureRunning());
    }
    if (_mdiArea->subWindowList().isEmpty() == true)
    {
        _mdiArea->setViewMode(QMdiArea::SubWindowView);
    }
    decorateTabs();
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
    if (childForm != nullptr)
    {
        emit childForm->triggerCopy();
    }
}

void MainWindow::on_actionPaste_triggered()
{
    ChildForm* childForm = getActiveChildWindow();
    if (childForm != nullptr)
    {
        emit childForm->triggerPaste();
    }
}

ChildForm* MainWindow::getActiveChildWindow()
{
    QMdiSubWindow* subWindow = _mdiArea->activeSubWindow();
    if (subWindow != nullptr)
    {
        ChildForm* childForm = dynamic_cast<ChildForm*>(subWindow->widget());
        return childForm;
    }
    return nullptr;
}

void MainWindow::on_actionFile_clipboard_triggered()
{
    if (_fileClipboarDock->isVisible() == true)
    {
        _fileClipboarDock->hide();
    }
    else
    {
        _fileClipboarDock->show();
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
    if (activeWindow != nullptr)
    {
        activeWindow->runProcess();
    }
}

void MainWindow::on_action_Clear_scrollback_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->clearScrollback();
    }
}

void MainWindow::on_actionSelect_All_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->selectAll();
    }
}

void MainWindow::on_action_Find_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->findText();
    }
}

void MainWindow::on_actionFind_next_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->findTextNext(false);
    }
}

void MainWindow::on_actionFind_prev_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->findTextNext(true);
    }
}

void MainWindow::on_actionFind_highlighted_text_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->findTextHighlighted();
    }
}

void MainWindow::subWindowStateChangedSlot(Qt::WindowStates oldState, Qt::WindowStates newState)
{
    (void)oldState;
    if (((newState & Qt::WindowMaximized) != 0) && (_mdiArea->viewMode() == QMdiArea::SubWindowView))
    {
        _mdiArea->setViewMode(QMdiArea::TabbedView);
        decorateTabs();
    }
}

// Draws the "restore to sub-window view" tab button glyph: a downward chevron
// (collapse arrow) in the given colour, sized/weighted to sit next to the tab's
// close button. Returns it as a QIcon ready to hand to a QToolButton.
static QIcon renderRestoreChevronIcon(const QColor& color, qreal devicePixelRatio)
{
    const int iconSide = 12;
    QPixmap pixmap(QSize(iconSide, iconSide) * devicePixelRatio);
    pixmap.setDevicePixelRatio(devicePixelRatio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(color);
    pen.setWidthF(1.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    const QPointF chevron[3] = { QPointF(3.0, 4.5), QPointF(6.0, 8.0), QPointF(9.0, 4.5) };
    painter.drawPolyline(chevron, 3);
    painter.end();

    return QIcon(pixmap);
}

void MainWindow::decorateTabs()
{
    QTabBar* tabBar = _mdiArea->findChild<QTabBar*>();
    if (tabBar != nullptr)
    {
        QList<QMdiSubWindow*> subWindows = _mdiArea->subWindowList(QMdiArea::CreationOrder);
        for (int i = 0; i < tabBar->count(); i++)
        {
            QWidget* existing = tabBar->tabButton(i, QTabBar::RightSide);
            bool decorated = ((existing != nullptr) && (existing->objectName() == "cbTabControls"));
            if ((decorated == false) && (i < subWindows.size()))
            {
                QMdiSubWindow* subWindow = subWindows.at(i);

                QWidget* controls = new QWidget(tabBar);
                controls->setObjectName("cbTabControls");
                QHBoxLayout* layout = new QHBoxLayout(controls);
                layout->setContentsMargins(0, 0, 0, 0);
                layout->setSpacing(0);

                QIcon restoreIcon = renderRestoreChevronIcon(tabBar->palette().color(QPalette::WindowText), devicePixelRatioF());
                QToolButton* restoreButton = new QToolButton(controls);
                restoreButton->setAutoRaise(true);
                restoreButton->setIcon(restoreIcon);
                QList<QSize> restoreIconSizes = restoreIcon.availableSizes();
                if (restoreIconSizes.isEmpty() == false)
                {
                    restoreButton->setIconSize(restoreIconSizes.first());
                }
                restoreButton->setToolTip(tr("Restore sub-window view"));
                connect(restoreButton, SIGNAL(clicked()), this, SLOT(restoreSubWindowViewSlot()));

                QToolButton* closeButton = new QToolButton(controls);
                closeButton->setAutoRaise(true);
                closeButton->setIconSize(QSize(12, 12));
                closeButton->setIcon(style()->standardIcon(QStyle::SP_TabCloseButton));
                closeButton->setToolTip(tr("Close window"));
                connect(closeButton, SIGNAL(clicked()), subWindow, SLOT(close()));

                layout->addWidget(restoreButton);
                layout->addWidget(closeButton);

                tabBar->setTabButton(i, QTabBar::RightSide, controls);
            }
        }
    }
}

void MainWindow::restoreSubWindowViewSlot()
{
    _mdiArea->setViewMode(QMdiArea::SubWindowView);
    QMdiSubWindow* activeSubWindow = _mdiArea->activeSubWindow();
    if (activeSubWindow != nullptr)
    {
        activeSubWindow->showNormal();
    }
}

void MainWindow::on_actionCapture_output_triggered()
{
    ChildForm* activeWindow = getActiveChildWindow();
    if (activeWindow != nullptr)
    {
        activeWindow->captureLog();
    }
}

void MainWindow::on_actionHelp_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/cdesjardins/ComBomb"));
}
