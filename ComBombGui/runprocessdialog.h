#ifndef RUNPROCESSDIALOG_H
#define RUNPROCESSDIALOG_H

#include "cbdialog.h"

namespace Ui {
class RunProcessDialog;
}

class RunProcessDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit RunProcessDialog(QWidget *parent = 0);
    virtual ~RunProcessDialog();
    QString getProgram();
    QString getWorkingDirectory();
    QStringList getArguments();
protected:
    virtual QString getSettingsRoot();
private slots:
    void on_programBrowseButton_clicked();
    void on_workingDirBrowseButton_clicked();
private:
    Ui::RunProcessDialog *ui;
};

#endif // RUNPROCESSDIALOG_H
