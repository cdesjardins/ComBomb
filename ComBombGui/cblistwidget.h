#ifndef CBLISTWIDGET_H
#define CBLISTWIDGET_H

#include <QListWidget>

class CBListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit CBListWidget(QWidget* parent = 0);

private slots:
    void itemChangedSlot(QListWidgetItem* changedItem);
private:
    void addBlankItem();
};

#endif // CBLISTWIDGET_H
