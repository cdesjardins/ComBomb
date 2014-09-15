#include "finddialog.h"
#include "ui_finddialog.h"
#include <QSettings>

#define CB_FIND_CASE_SENSITIVE          getSettingsRoot() + "/CaseSensitive"
#define CB_FIND_SEARCH_UP               getSettingsRoot() + "/SearchUp"

FindDialog::FindDialog(QWidget *parent) :
    CBDialog(parent),
    ui(new Ui::FindDialog)
{
    QSettings settings;
    ui->setupUi(this);
    ui->caseSensitiveCheckBox->setChecked(settings.value(CB_FIND_CASE_SENSITIVE, false).toBool());
    ui->searchUpCheckBox->setChecked(settings.value(CB_FIND_SEARCH_UP, false).toBool());
    ui->findWhatComboBox->restoreComboBox();
}

FindDialog::~FindDialog()
{
    QSettings settings;
    settings.setValue(CB_FIND_CASE_SENSITIVE, ui->caseSensitiveCheckBox->isChecked());
    settings.setValue(CB_FIND_SEARCH_UP, ui->searchUpCheckBox->isChecked());
    ui->findWhatComboBox->saveComboBox();
    delete ui;
}

QString FindDialog::getString()
{
    return ui->findWhatComboBox->currentText();
}

void FindDialog::addString(const QString& newStr)
{
    ui->findWhatComboBox->setCurrentText(newStr);
}

bool FindDialog::getCaseSensitivity()
{
    return ui->caseSensitiveCheckBox->isChecked();
}

bool FindDialog::getSearchUp()
{
    return ui->searchUpCheckBox->isChecked();
}

QString FindDialog::getSettingsRoot()
{
    return objectName();
}
