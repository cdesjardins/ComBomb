#include "cblistwidget.h"

CBListWidget::CBListWidget(QWidget * parent)
    : QListWidget(parent)
{
    addBlankItem();
    connect(this, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChangedSlot(QListWidgetItem*)));
}

void CBListWidget::itemChangedSlot(QListWidgetItem* changedItem)
{
    QListWidgetItem* lastItem = item (count() - 1);
    if (lastItem == changedItem)
    {
        if (changedItem->text().length() > 0)
        {
            addBlankItem();
        }
    }
    else
    {
        if (changedItem->text().length() == 0)
        {
            delete takeItem(row(changedItem));
        }
    }
}

void CBListWidget::addBlankItem()
{
    QListWidgetItem* newBlankItem;
    newBlankItem = new QListWidgetItem("");
    newBlankItem->setFlags(newBlankItem->flags () | Qt::ItemIsEditable);
    addItem(newBlankItem);
}
