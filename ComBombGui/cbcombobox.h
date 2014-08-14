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
    void setDefault(const QString &defaultVal);
signals:

public slots:

protected:
    virtual void showEvent(QShowEvent* event);
    void CBComboBox::hideEvent(QHideEvent* event);
    QString getName();
    void saveComboBox();
    void saveEditableComboBox();
    void saveStaticComboBox();

    void restoreComboBox();
    void restoreEditableComboBox();
    void restoreStaticComboBox();
private:
    QString _default;
};

#endif // CBCOMBOBOX_H
