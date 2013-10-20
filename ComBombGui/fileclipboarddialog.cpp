#include <QSettings>
#include "fileclipboarddialog.h"
#include "ui_fileclipboarddialog.h"
#include "mainwindow.h"

#define CB_FILE_CLIBBOARD_SETTINGS_ROOT "FileClipboard/"

FileClipboardDialog::FileClipboardDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::FileClipboardDialog),
    _fileClipboardHeader(NULL)
{
    ui->setupUi(this);
    QSettings settings;
    QTableWidgetItem* item;

    settings.beginReadArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    for (int row = 0; row < 256; row++)
    {
        settings.setArrayIndex(row);
        item = new QTableWidgetItem(settings.value("Text").toString());
        ui->fileClipboardTable->setItem(row, 0, item);
    }

    settings.endArray();
    _fileClipboardHeader = new FileClipboardHeader();
    ui->fileClipboardTable->setVerticalHeader(_fileClipboardHeader);
    bool sendNewLineChecked = settings.value(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine").toBool();
    ui->newLineCheckBox->setChecked(sendNewLineChecked);
    connect(_fileClipboardHeader, SIGNAL(sendItemSignal(int)), this, SLOT(sendItemTriggered(int)));
    ui->fileClipboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

FileClipboardDialog::~FileClipboardDialog()
{
    QTableWidgetItem* item;
    QSettings settings;
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }
    settings.beginWriteArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    for (int row = 0; row < ui->fileClipboardTable->rowCount(); row++)
    {
        settings.setArrayIndex(row);
        item = ui->fileClipboardTable->takeItem(row, 0);
        settings.setValue("Text", item->text());
        delete item;
    }
    settings.endArray();
    settings.setValue(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine", ui->newLineCheckBox->isChecked());

    delete ui;
    if (_fileClipboardHeader != NULL)
    {
        delete _fileClipboardHeader;
    }
}

void FileClipboardDialog::sendItemTriggered(int index)
{
    QTableWidgetItem* item = ui->fileClipboardTable->item(index, 0);
    if (item->text().length() > 0)
    {
        ChildForm* c = MainWindow::getMainWindow()->getActiveChildWindow();
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

void FileClipboardDialog::hideEvent(QHideEvent*)
{
    MainWindow::saveWidgetGeometry(this, CB_FILE_CLIBBOARD_SETTINGS_ROOT "Geometry");
}

void FileClipboardDialog::showEvent(QShowEvent*)
{
    MainWindow::restoreWidgetGeometry(this, CB_FILE_CLIBBOARD_SETTINGS_ROOT "Geometry");
}

