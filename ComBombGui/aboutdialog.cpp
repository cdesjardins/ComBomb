#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "versioning.h"
#include "cblabel.h"

CBLabel::CBLabel(QWidget *parent)
    : QLabel(parent),
      _qImg(NULL)
{

}

CBLabel::~CBLabel()
{
    if (_qImg != NULL)
    {
        delete _qImg;
    }
}

void CBLabel::mouseDoubleClickEvent(QMouseEvent *)
{
    if (_qImg == NULL)
    {
        _qImg = new QImage();
        _qImg->load(":/images/myicon.png");
        QPixmap pixma = QPixmap::fromImage(*_qImg);
        setPixmap(pixma);
    }
}


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    QString ver("ComBomb version: ");
    ver.append(getVersion());
    ui->versionLabel->setText(ver);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
