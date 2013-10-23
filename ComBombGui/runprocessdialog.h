#ifndef RUNPROCESSDIALOG_H
#define RUNPROCESSDIALOG_H

#include <QDialog>

namespace Ui {
class RunProcessDialog;
}

class RunProcessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunProcessDialog(QWidget *parent = 0);
    ~RunProcessDialog();

private slots:
    void on_programBrowseButton_clicked();

    void on_workingDirBrowseButton_clicked();

private:
    Ui::RunProcessDialog *ui;
};

#endif // RUNPROCESSDIALOG_H
