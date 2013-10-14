#include <QSettings>
#include "fileclipboarddialog.h"
#include "fileclipboardheader.h"
#include "ui_fileclipboarddialog.h"




FileClipboardDialog::FileClipboardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileClipboardDialog)
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
    ui->fileClipboardTable->setVerticalHeader(new FileClipboardHeader());
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
}


void FileClipboardDialog::on_fileClipboardTable_itemDoubleClicked(QTableWidgetItem *item)
{
    qDebug("item double clicked %s", item->text());
}

void FileClipboardDialog::on_fileClipboardTable_cellDoubleClicked(int row, int column)
{
    qDebug("cell double clicked %d %d", row, column);
}

