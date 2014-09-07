#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include "cbdialog.h"

namespace Ui {
class FindDialog;
}

class FindDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();
    QString getString();
    bool getCaseSensitivity();
    bool getSearchUp();
    virtual QString getSettingsRoot();
private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
