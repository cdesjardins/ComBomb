#include <QSettings>
#include "fileclipboarddialog.h"
#include "ui_fileclipboarddialog.h"
#include "mainwindow.h"

FileClipboardDialog::FileClipboardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileClipboardDialog),
    _fileClipboardHeader(NULL)
{
    ui->setupUi(this);
    QSettings settings;
    QTableWidgetItem *item;

    settings.beginReadArray("FileClipboard");
    for (int row = 0; row < 256; row++)
    {
        settings.setArrayIndex(row);
        item = new QTableWidgetItem(settings.value("Text").toString());
        ui->fileClipboardTable->setItem(row, 0, item);
    }

    settings.endArray();
    _fileClipboardHeader = new FileClipboardHeader();
    ui->fileClipboardTable->setVerticalHeader(_fileClipboardHeader);
    connect(_fileClipboardHeader, SIGNAL(sendItemSignal(int)), this, SLOT(sendItemTriggered(int)));
}

FileClipboardDialog::~FileClipboardDialog()
{
    QTableWidgetItem *item;
    QSettings settings;
    settings.beginWriteArray("FileClipboard");
    for (int row = 0; row < ui->fileClipboardTable->rowCount(); row++)
    {
        settings.setArrayIndex(row);
        item = ui->fileClipboardTable->takeItem(row, 0);
        settings.setValue("Text", item->text());
        delete item;
    }
    settings.endArray();
    delete ui;
    if (_fileClipboardHeader != NULL)
    {
        delete _fileClipboardHeader;
    }
}


void FileClipboardDialog::sendItemTriggered(int index)
{
    QTableWidgetItem * item = ui->fileClipboardTable->item(index, 0);
    if (item->text().length() > 0)
    {
        //qDebug("Send %s", item->text().toLocal8Bit().constData());
        ChildForm *c = MainWindow::getMainWindow()->getActiveChildWindow();
        if (c != NULL)
        {
            QString text = item->text();
            if (ui->newLineCheckBox->isChecked())
            {
                text += "\n";
            }
            c->sendText(text);
        }
    }
}

void FileClipboardDialog::hideEvent(QHideEvent *)
{
    qDebug("hideEvent");
    MainWindow::saveWidgetGeometry(this, "FileClipboardGeometry");
}

void FileClipboardDialog::showEvent(QShowEvent *)
{
    qDebug("showEvent");
    MainWindow::restoreWidgetGeometry(this, "FileClipboardGeometry");
}


