#ifndef CBDIALOG_H
#define CBDIALOG_H

#include <QDialog>

class CBDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CBDialog(QWidget *parent = 0);
    virtual ~CBDialog() {}
protected:
    virtual QString getSettingsRoot() = 0;
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

signals:

public slots:

};

#endif // CBDIALOG_H
