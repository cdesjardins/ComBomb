#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"

#define CB_CONFIG_SETTINGS_ROOT "ConfigDialog/"
#define CB_CONFIG_SETTINGS_COM "ConfigDialog/ComPorts"
#define CB_CONFIG_SETTINGS_PORT "Port"
#ifdef WIN32
#define BASE_PORTNAME_1 "COM", 1, 257
#else
#define BASE_PORTNAME_1 "/dev/ttyS", 0, 256
#define BASE_PORTNAME_2 "/dev/ttyUSB", 0, 256
#endif


ConfigDialog::ConfigDialog(QWidget *parent) :
    CBDialog(parent),
    ui(new Ui::ConfigDialog)
{
    QTerminalConfig terminalConfig;
    ui->setupUi(this);

    if (ConfigDialog::getTerminalConfig(&terminalConfig) == true)
    {
        ui->fontComboBox->setCurrentFont(terminalConfig._font);
    }
    ui->wordSelectionDelimitersLineEdit->setText(terminalConfig._wordSelectionDelimiters);
    for (int fontSize = 6; fontSize < 20; fontSize++)
    {
        std::stringstream fontSizeStr;
        fontSizeStr << fontSize;
        ui->fontSizeComboBox->addItem(fontSizeStr.str().c_str(), fontSize);
    }
    ui->fontSizeComboBox->setDefault("12");
    populateComPortListWidget();
}

void ConfigDialog::setPortListSettings()
{
    QSettings settings;
    QListWidgetItem* item;

    settings.beginWriteArray(CB_CONFIG_SETTINGS_ROOT);
    for (int row = 0; row < ui->comPortsList->count(); row++)
    {
        settings.setArrayIndex(row);
        item = ui->comPortsList->item(row);
        settings.setValue(CB_CONFIG_SETTINGS_PORT, item->text());
    }
    settings.endArray();
}

QStringList ConfigDialog::getPortListSettings()
{
    QStringList comPorts;
    int row = 0;
    QSettings settings;
    QString port;
    settings.beginReadArray(CB_CONFIG_SETTINGS_ROOT);
    do
    {
        settings.setArrayIndex(row++);
        port = settings.value(CB_CONFIG_SETTINGS_PORT).toString();
        if (port.length() > 0)
        {
            comPorts.append(port);
        }
    } while (port.length() > 0);
    settings.endArray();
    if (comPorts.length() == 0)
    {
        comPorts = ConfigDialog::getPortListDefaults(BASE_PORTNAME_1);
#ifdef BASE_PORTNAME_2
        comPorts += ConfigDialog::getPortListDefaults(BASE_PORTNAME_2);
#endif
    }
    return comPorts;
}

QStringList ConfigDialog::getPortListDefaults(QString basePortName, int start, int stop)
{
    QStringList comPorts;

    for (int i = start; i < stop; i++)
    {
        comPorts.append(basePortName + QString::number(i));
    }
    return comPorts;
}

void ConfigDialog::populateComPortListWidget()
{
    QListWidgetItem* item;
    int row = 0;
    QStringList comPorts = ConfigDialog::getPortListSettings();
    for (QStringList::iterator it = comPorts.begin(); it != comPorts.end(); ++it)
    {
        item = new QListWidgetItem(*it);
        item->setFlags(item->flags () | Qt::ItemIsEditable);
        ui->comPortsList->insertItem(row++, item);
    }
}

ConfigDialog::~ConfigDialog()
{
    if (isHidden() == false)
    {
        hideEvent(NULL);
    }
    delete ui;
}

QString ConfigDialog::getSettingsRoot()
{
    return CB_CONFIG_SETTINGS_ROOT;
}

void ConfigDialog::on_buttonBox_accepted()
{
    QByteArray qbytes;
    QSettings settings;
    QTerminalConfig terminalConfig;
    QDataStream q(&qbytes, QIODevice::WriteOnly);
    terminalConfig._wordSelectionDelimiters = ui->wordSelectionDelimitersLineEdit->text().toLocal8Bit().constData();
    terminalConfig._font = ui->fontComboBox->currentFont();
    terminalConfig._font.setPointSize(ui->fontSizeComboBox->currentData().toInt());
    q << terminalConfig;

    settings.setValue(CB_CONFIG_SETTINGS_ROOT "Settings", qbytes);
    setPortListSettings();

    QList<ChildForm*> list = MainWindow::getMainWindow()->findChildren<ChildForm *>();
    foreach(ChildForm *w, list)
    {
        w->applyTerminalConfig(terminalConfig);
    }

}

bool ConfigDialog::getTerminalConfig(QTerminalConfig *terminalConfig)
{
    bool ret = false;
    QSettings settings;
    QByteArray qbytes(settings.value(CB_CONFIG_SETTINGS_ROOT "Settings").toByteArray());
    if (qbytes.length() > 0)
    {
        QDataStream q(qbytes);
        q >> *terminalConfig;
        ret = true;
    }
    return ret;
}


