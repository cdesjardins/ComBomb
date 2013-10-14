#ifndef FILECLIPBOARDDIALOG_H
#define FILECLIPBOARDDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class FileClipboardDialog;
}

class FileClipboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileClipboardDialog(QWidget *parent = 0);
    ~FileClipboardDialog();

private slots:
    void on_fileClipboardTable_itemDoubleClicked(QTableWidgetItem *item);
    void on_fileClipboardTable_cellDoubleClicked(int row, int column);
private:
    Ui::FileClipboardDialog *ui;
};

#endif // FILECLIPBOARDDIALOG_H
