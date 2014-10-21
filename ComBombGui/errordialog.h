#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include "cbdialog.h"
#include <QTextEdit>

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public CBDialog
{
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget *parent = 0);
    virtual ~ErrorDialog();
    int addTab(QString title);
    void removeTab(int index);
    void addText(int index, QString text);
    void clearText(int index);
    void setFont(QFont font);
protected:
    virtual QString getSettingsRoot();
private slots:
    void changeWidget(int comboBoxIndex);
    void on_pushButton_clicked();

private:
    QTextEdit* getEditor(int index);
    Ui::ErrorDialog *ui;
    std::map<int, QTextEdit*> _labelWidgets;
    int _labelMapKey;
};

#endif // ERRORDIALOG_H
