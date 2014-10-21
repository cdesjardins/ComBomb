#include "errordialog.h"
#include "configdialog.h"
#include "ui_errordialog.h"
#include "QTerminal/QTerminalConfig.h"
#include <QScrollBar>

#define CB_ERRORBOX_SETTINGS_ROOT "ErrorBox/"

ErrorDialog::ErrorDialog(QWidget *parent) :
    CBDialog(parent),
    ui(new Ui::ErrorDialog),
    _labelMapKey(0)
{
    ui->setupUi(this);
    connect(ui->selectorComboBox, SIGNAL(activated(int)), this, SLOT(changeWidget(int)));
}

ErrorDialog::~ErrorDialog()
{
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }
    Ui::ErrorDialog* tmp = ui;
    ui = NULL;
    delete tmp;
    std::map<int, QTextEdit *>::iterator it;
    for (it = _labelWidgets.begin(); it != _labelWidgets.end(); it++)
    {
        QTextEdit* plainTextEdit = it->second;
        delete plainTextEdit;
    }
    _labelWidgets.clear();
}

QString ErrorDialog::getSettingsRoot()
{
    return CB_ERRORBOX_SETTINGS_ROOT;
}

void ErrorDialog::changeWidget(int comboBoxIndex)
{
    int index = ui->selectorComboBox->itemData(comboBoxIndex).toInt();
    QTextEdit *plainTextEdit = getEditor(index);
    if (plainTextEdit != NULL)
    {
        ui->stackedWidget->setCurrentWidget(plainTextEdit);
    }
}

QTextEdit *ErrorDialog::getEditor(int index)
{
    std::map<int, QTextEdit *>::iterator it = _labelWidgets.find(index);
    if (it != _labelWidgets.end())
    {
        QTextEdit* plainTextEdit = it->second;
        return plainTextEdit;
    }
    return NULL;
}

int ErrorDialog::addTab(QString title)
{
    QTextEdit *plainTextEdit = new QTextEdit(this);
    _labelMapKey++;
    if (plainTextEdit != NULL)
    {
        QTerminalConfig terminalConfig;
        ConfigDialog::getTerminalConfig(&terminalConfig);

        plainTextEdit->setReadOnly(true);
        plainTextEdit->setFont(terminalConfig._font);
        _labelWidgets.insert(std::pair<int, QTextEdit*>(_labelMapKey, plainTextEdit));
        ui->stackedWidget->addWidget(plainTextEdit);
        ui->selectorComboBox->addItem(title, _labelMapKey);
    }
    return _labelMapKey;
}

void ErrorDialog::removeTab(int index)
{
    QTextEdit* plainTextEdit = getEditor(index);
    if ((ui != NULL) && (plainTextEdit != NULL))
    {
        ui->stackedWidget->removeWidget(plainTextEdit);
        int comboBoxIndex = ui->selectorComboBox->findData(index);
        if (comboBoxIndex != -1)
        {
            ui->selectorComboBox->removeItem(comboBoxIndex);
        }
    }
}

void ErrorDialog::addText(int index, QString text)
{
    QTextEdit* plainTextEdit = getEditor(index);
    if (plainTextEdit != NULL)
    {
        QScrollBar *sb = plainTextEdit->verticalScrollBar();
        const bool atEndOfOutput = (sb->value() == sb->maximum());
        plainTextEdit->insertPlainText(text);
        if (atEndOfOutput == true)
        {
            sb->setValue(sb->maximum());
        }
    }
}

void ErrorDialog::clearText(int index)
{
    QTextEdit* plainTextEdit = getEditor(index);
    if (plainTextEdit != NULL)
    {
        plainTextEdit->clear();
    }
}

void ErrorDialog::on_pushButton_clicked()
{
    int index = ui->selectorComboBox->currentData().toInt();
    clearText(index);
}

void ErrorDialog::setFont(QFont font)
{
    std::map<int, QTextEdit *>::iterator it;
    for (it = _labelWidgets.begin(); it != _labelWidgets.end(); it++)
    {
        QTextEdit* plainTextEdit = it->second;
        plainTextEdit->setFont(font);
    }
}
