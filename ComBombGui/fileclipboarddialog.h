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
    explicit FileClipboardDialog(QWidget *parent = 0);
    ~FileClipboardDialog();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private slots:
    void sendItemTriggered(int index);

private:
    Ui::FileClipboardDialog *ui;
    FileClipboardHeader *_fileClipboardHeader;
};

#endif // FILECLIPBOARDDIALOG_H
