#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "QTerminal/QTerminalConfig.h"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

    static void getTerminalConfig(QTerminalConfig *terminalConfig);
protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
