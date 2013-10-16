#ifndef CBLABEL_H
#define CBLABEL_H

#include <QLabel>

class CBLabel : public QLabel
{
    Q_OBJECT
public:
    CBLabel(QWidget* parent = 0);
    ~CBLabel();
    void mouseDoubleClickEvent(QMouseEvent* event);
private:
    QImage* _qImg;
};

#endif // CBLABEL_H
