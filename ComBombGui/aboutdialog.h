#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "cbdialog.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = 0);
    ~AboutDialog();

protected:
    virtual QString getSettingsRoot();

private:
    Ui::AboutDialog* ui;
};

#endif // ABOUTDIALOG_H
