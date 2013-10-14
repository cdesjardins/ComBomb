#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QCoreApplication::setOrganizationName("Desjardins");
    QCoreApplication::setOrganizationDomain("chrisd.info");
    QCoreApplication::setApplicationName("ComBomb");

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}

