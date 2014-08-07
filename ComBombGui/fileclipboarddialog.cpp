#include <QSettings>
#include "fileclipboarddialog.h"
#include "ui_fileclipboarddialog.h"
#include "mainwindow.h"

#define CB_FILE_CLIBBOARD_SETTINGS_ROOT "FileClipboard/"

FileClipboardDialog::FileClipboardDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::FileClipboardDialog),
    _fileClipboardHeader(NULL),
    _fileClipboardLoaded(false)
{
    ui->setupUi(this);

    loadFileClipboardSettings();
    loadNewLineSettings();

    _fileClipboardHeader = new FileClipboardHeader();
    ui->fileClipboardTable->setVerticalHeader(_fileClipboardHeader);

    connect(_fileClipboardHeader, SIGNAL(sendItemSignal(int)), this, SLOT(sendItemTriggered(int)));
    ui->fileClipboardTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _fileClipboardLoaded = true;
}

FileClipboardDialog::~FileClipboardDialog()
{
    QSettings settings;
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }

    delete ui;
    if (_fileClipboardHeader != NULL)
    {
        delete _fileClipboardHeader;
    }
}

void FileClipboardDialog::saveFileClipboardSetting(int row)
{
    QTableWidgetItem* item;
    QSettings settings;
    settings.beginWriteArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    settings.setArrayIndex(row);
    item = ui->fileClipboardTable->item(row, 0);
    settings.setValue("Text", item->text());
    settings.endArray();
}

void FileClipboardDialog::loadFileClipboardSettings()
{
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
            QApplication::setActiveWindow(MainWindow::getMainWindow());
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

void FileClipboardDialog::on_fileClipboardTable_cellChanged(int row, int column)
{
    UNREF_PARAM(column);
    if (_fileClipboardLoaded == true)
    {
        saveFileClipboardSetting(row);
    }
}

void FileClipboardDialog::on_newLineCheckBox_toggled(bool checked)
{
    QSettings settings;
    settings.setValue(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine", checked);
}

void FileClipboardDialog::loadNewLineSettings()
{
    QSettings settings;
    bool sendNewLineChecked = settings.value(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine").toBool();
    ui->newLineCheckBox->setChecked(sendNewLineChecked);
}
