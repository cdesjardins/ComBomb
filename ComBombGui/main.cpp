#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("Desjardins");
    QCoreApplication::setOrganizationDomain("chrisd.info");
    QCoreApplication::setApplicationName("ComBomb");

    QApplication a(argc, argv);

    MainWindow *w = MainWindow::getMainWindow();
    w->show();
    int ret = a.exec();

    MainWindow::destroyMainWindow();
    return ret;
}

