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
    QString name = getName();
    settings.beginWriteArray(name);
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
    QString name = getName();
    int size = settings.beginReadArray(name);
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
    QString tmpItem;

    if (index == -1)
    {
        tmpItem = item;
    }
    else
    {
        tmpItem = itemText(index);
        removeItem(index);
    }
    insertItem(0, tmpItem);
    setCurrentIndex(0);
}

