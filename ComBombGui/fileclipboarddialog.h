#ifndef FILECLIPBOARDDIALOG_H
#define FILECLIPBOARDDIALOG_H

#include "cbdialog.h"
#include "fileclipboardheader.h"

namespace Ui {
class FileClipboardDialog;
}

class FileClipboardDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit FileClipboardDialog(QWidget* parent = 0);
    ~FileClipboardDialog();

protected:
    void loadFileClipboardSettings();
    void saveFileClipboardSetting(int row);
    void loadNewLineSettings();
    virtual QString getSettingsRoot();
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
