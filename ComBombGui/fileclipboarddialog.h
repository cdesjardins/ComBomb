#ifndef FILECLIPBOARDDIALOG_H
#define FILECLIPBOARDDIALOG_H

#include <QDialog>
#include "fileclipboardheader.h"

namespace Ui {
class FileClipboardDialog;
}

class FileClipboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileClipboardDialog(QWidget* parent = 0);
    ~FileClipboardDialog();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);
    void loadFileClipboardSettings();
    void saveFileClipboardSetting(int row);
    void loadNewLineSettings();
private slots:
    void sendItemTriggered(int index);

    void on_fileClipboardTable_cellChanged(int row, int column);

    void on_newLineCheckBox_toggled(bool checked);

private:
    Ui::FileClipboardDialog* ui;
    FileClipboardHeader* _fileClipboardHeader;
    bool _fileClipboardLoaded;
};

#endif // FILECLIPBOARDDIALOG_H
