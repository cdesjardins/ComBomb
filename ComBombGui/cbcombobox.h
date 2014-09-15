#ifndef CBCOMBOBOX_H
#define CBCOMBOBOX_H

#include <QComboBox>

class CBComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CBComboBox(QWidget *parent = 0);
    virtual ~CBComboBox();
    void addOrUpdateItem(const QString &item);
    void setDefault(const QString &defaultVal);
    void restoreComboBox();
    void saveComboBox();
signals:

public slots:

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);
    QString getName();
    void saveEditableComboBox();
    void saveStaticComboBox();

    void restoreEditableComboBox();
    void restoreStaticComboBox();
private:
    QString _default;
    bool _saved;
    bool _restored;
};

#endif // CBCOMBOBOX_H
