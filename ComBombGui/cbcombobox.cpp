#include "cbcombobox.h"
#include <QSettings>

CBComboBox::CBComboBox(QWidget *parent) :
    QComboBox(parent)
{
}

CBComboBox::~CBComboBox()
{
    saveComboBox();
}

QString CBComboBox::getName()
{
    QString ret(parentWidget()->objectName() + "/" + objectName());
    return ret;
}

void CBComboBox::showEvent(QShowEvent *)
{
    restoreComboBox();
}

void CBComboBox::saveComboBox()
{
    QSettings settings;
    QStringList itemList;
    int i;
    QString cur = currentText();
    if (cur.length() > 0)
    {
        itemList.append(cur);
    }
    for (i = 0; i < count(); i++)
    {
        itemList.append(itemText(i));
    }
    itemList.removeDuplicates();
    settings.beginWriteArray(getName());
    i = 0;
    for (QStringList::iterator it = itemList.begin(); it != itemList.end(); it++, i++)
    {
        settings.setArrayIndex(i);
        settings.setValue("Item", *it);
    }
    settings.endArray();
}

void CBComboBox::restoreComboBox()
{
    QSettings settings;
    int size = settings.beginReadArray(getName());
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        addItem(settings.value("Item").toString());
    }
    settings.endArray();
}

void CBComboBox::addOrUpdateItem(const QString &item)
{
    int index = findText(item);
    if (index == -1)
    {
        insertItem(0, item);
        setCurrentIndex(0);
    }
    else
    {
        bump(index);
    }
}

void CBComboBox::bump(int index)
{
    QString tmpItem;
    tmpItem = itemText(index);
    removeItem(index);
    insertItem(0, tmpItem);
    setCurrentIndex(0);
}
