#include "cbdialog.h"
#include <QSettings>

CBDialog::CBDialog(QWidget *parent) :
    QDialog(parent)
{
}
void CBDialog::showEvent(QShowEvent* )
{
    QSettings settings;
    restoreGeometry(settings.value(getSettingsRoot() + "Geometry").toByteArray());
}

void CBDialog::hideEvent(QHideEvent* )
{
    QSettings settings;
    settings.setValue(getSettingsRoot() + "Geometry", saveGeometry());
}

