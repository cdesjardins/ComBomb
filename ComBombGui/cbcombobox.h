#ifndef CBCOMBOBOX_H
#define CBCOMBOBOX_H

#include <QComboBox>

class CBComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CBComboBox(QWidget *parent = 0);
    ~CBComboBox();
    void addOrUpdateItem(const QString &item);
    void bump(int index);
signals:

public slots:

protected:
    virtual void showEvent(QShowEvent * event);
    QString getName();
    void saveComboBox();
    void restoreComboBox();
};

#endif // CBCOMBOBOX_H
