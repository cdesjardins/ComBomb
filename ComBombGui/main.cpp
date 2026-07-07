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
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    // On a Wayland session Qt picks the wayland platform plugin, which cannot
    // move or resize a floating QDockWidget (the compositor owns window
    // placement, so the drag is silently rejected and resizing comes from the
    // wrong edge). Force xcb (via XWayland) so docks behave like X11/Windows.
    // An explicit QT_QPA_PLATFORM in the environment still overrides this.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }

    QCoreApplication::setOrganizationName("Desjardins");
    QCoreApplication::setOrganizationDomain("chrisd.info");
    QCoreApplication::setApplicationName("ComBomb");

    QApplication a(argc, argv);

    MainWindow* w = MainWindow::getMainWindow();
    w->show();
    int ret = a.exec();

    MainWindow::destroyMainWindow();
    return ret;
}
