#include "cryptlib.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    int ret;
    QApplication a(argc, argv);
    MainWindow w;

    int status = cryptInit();
    if (cryptStatusError(status))
    {
        QMessageBox msgBox;
        msgBox.setText("Unable to init crypt lib");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    else
    {
        w.show();
    }
    ret = a.exec();
    cryptEnd();
    return ret;
}
