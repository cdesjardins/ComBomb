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

    static bool getTerminalConfig(QTerminalConfig *terminalConfig);
    static QStringList getPortListSettings();
protected:
    static QStringList getPortListDefaults(QString basePortName, int start, int stop);
    void setPortListSettings();
    void populateComPortListWidget();
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
