#ifndef FILECLIPBOARDHEADER_H
#define FILECLIPBOARDHEADER_H
#include <QHeaderView>

class FileClipboardHeader : public QHeaderView
{
    Q_OBJECT
public:
    FileClipboardHeader()
        : QHeaderView(Qt::Vertical)
    {
        setSectionsClickable(true);
        connect(this, SIGNAL(sectionClicked(int)), this, SLOT(sectionClicked(int)));
    }
    ~FileClipboardHeader()
    {

    }

public slots:
    void sectionClicked(int index)
    {
        qDebug("Index: %i", index);
    }
};
#endif // FILECLIPBOARDHEADER_H
