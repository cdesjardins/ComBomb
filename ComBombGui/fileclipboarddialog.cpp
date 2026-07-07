/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    https://github.com/cdesjardins/ComBomb cjd@chrisd.info

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <algorithm>
#include <QSettings>
#include <QTableWidget>
#include <QHeaderView>
#include <QToolButton>
#include <QLineEdit>
#include <QTabBar>
#include <QApplication>
#include "fileclipboarddialog.h"
#include "ui_fileclipboarddialog.h"
#include "mainwindow.h"

#define CB_FILE_CLIBBOARD_SETTINGS_ROOT "FileClipboard/"
#define CB_FILE_CLIPBOARD_OPEN_SLOTS    CB_FILE_CLIBBOARD_SETTINGS_ROOT "OpenSlots"
#define CB_FILE_CLIPBOARD_CURRENT_SLOT  CB_FILE_CLIBBOARD_SETTINGS_ROOT "CurrentSlot"

namespace
{
// Per-slot QSettings keys. A slot keeps its name and entries even while the
// tab is closed, so reopening the slot rehydrates whatever was last there.
QString slotNameKey(int slot)
{
    return QString(CB_FILE_CLIBBOARD_SETTINGS_ROOT "Tab/%1/Name").arg(slot);
}

QString slotEntriesArray(int slot)
{
    return QString(CB_FILE_CLIBBOARD_SETTINGS_ROOT "Tab/%1/Entries").arg(slot);
}

QList<int> parseSlotList(const QString& packed)
{
    QList<int> slotList;
    const QStringList parts = packed.split(',', Qt::SkipEmptyParts);
    for (const QString& part : parts)
    {
        bool ok = false;
        int value = part.trimmed().toInt(&ok);
        if (ok == true)
        {
            slotList.append(value);
        }
    }
    std::sort(slotList.begin(), slotList.end());
    return slotList;
}
}

FileClipboardDialog::FileClipboardDialog(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::FileClipboardDialog),
    _tabsLoaded(false),
    _renameEditor(nullptr),
    _renameSlot(-1)
{
    ui->setupUi(this);

    ui->tabWidget->setTabsClosable(true);
    ui->tabWidget->setMovable(false);

    QToolButton* addButton = new QToolButton(this);
    addButton->setText("+");
    addButton->setAutoRaise(true);
    addButton->setToolTip(tr("Add a new clipboard tab"));
    connect(addButton, &QToolButton::clicked, this, &FileClipboardDialog::onAddTabClicked);
    ui->tabWidget->setCornerWidget(addButton, Qt::TopRightCorner);

    connect(ui->tabWidget, &QTabWidget::tabCloseRequested, this, &FileClipboardDialog::onTabCloseRequested);
    connect(ui->tabWidget, &QTabWidget::tabBarDoubleClicked, this, &FileClipboardDialog::onTabDoubleClicked);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &FileClipboardDialog::onCurrentTabChanged);

    loadTabs();
    loadNewLineSettings();

    _tabsLoaded = true;
    ui->searchComboBox->installEventFilter(this);
}

FileClipboardDialog::~FileClipboardDialog()
{
    if (isHidden() == false)
    {
        hideEvent(nullptr);
    }

    delete ui;
}

QTableWidget* FileClipboardDialog::createTableForSlot(int slot)
{
    QTableWidget* table = new QTableWidget(kRowCount, 1, ui->tabWidget);
    table->setProperty("slot", slot);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setVisible(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    FileClipboardHeader* header = new FileClipboardHeader();
    table->setVerticalHeader(header);
    connect(header, &FileClipboardHeader::sendItemSignal, this, [this, table](int index)
    {
        sendItemTriggered(table, index);
    });

    // Populate from settings before connecting cellChanged so loading does not
    // trigger spurious saves.
    QSettings settings;
    settings.beginReadArray(slotEntriesArray(slot));
    for (int row = 0; row < kRowCount; row++)
    {
        settings.setArrayIndex(row);
        QTableWidgetItem* item = new QTableWidgetItem(settings.value("Text").toString());
        table->setItem(row, 0, item);
    }
    settings.endArray();

    connect(table, &QTableWidget::cellChanged, this, &FileClipboardDialog::onTableCellChanged);
    return table;
}

QTableWidget* FileClipboardDialog::addTabForSlot(int slot)
{
    QSettings settings;
    QString name = settings.value(slotNameKey(slot)).toString();
    if (name.isEmpty() == true)
    {
        name = tr("Tab %1").arg(slot + 1);
        settings.setValue(slotNameKey(slot), name);
    }

    QTableWidget* table = createTableForSlot(slot);

    // Keep tabs ordered by ascending slot number so a reopened slot lands back
    // in its original position.
    int pos = 0;
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        if (ui->tabWidget->widget(i)->property("slot").toInt() < slot)
        {
            pos++;
        }
    }
    ui->tabWidget->insertTab(pos, table, name);
    return table;
}

QTableWidget* FileClipboardDialog::currentTable() const
{
    return qobject_cast<QTableWidget*>(ui->tabWidget->currentWidget());
}

QList<int> FileClipboardDialog::openSlots() const
{
    QList<int> slotList;
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        slotList.append(ui->tabWidget->widget(i)->property("slot").toInt());
    }
    std::sort(slotList.begin(), slotList.end());
    return slotList;
}

int FileClipboardDialog::lowestFreeSlot() const
{
    QList<int> slotList = openSlots();
    int slot = 0;
    while (slotList.contains(slot) == true)
    {
        slot++;
    }
    return slot;
}

void FileClipboardDialog::saveOpenSlots()
{
    QStringList packed;
    const QList<int> slotList = openSlots();
    for (int slot : slotList)
    {
        packed.append(QString::number(slot));
    }
    QSettings settings;
    settings.setValue(CB_FILE_CLIPBOARD_OPEN_SLOTS, packed.join(','));
}

void FileClipboardDialog::loadTabs()
{
    QSettings settings;
    if (settings.contains(CB_FILE_CLIPBOARD_OPEN_SLOTS) == false)
    {
        migrateLegacyClipboard();
    }

    QList<int> slotList = parseSlotList(settings.value(CB_FILE_CLIPBOARD_OPEN_SLOTS).toString());
    if (slotList.isEmpty() == true)
    {
        slotList.append(0);
    }

    for (int slot : slotList)
    {
        addTabForSlot(slot);
    }

    // Restore the tab that was selected last session.
    int currentSlot = settings.value(CB_FILE_CLIPBOARD_CURRENT_SLOT, slotList.first()).toInt();
    int currentPos = tabPosForSlot(currentSlot);
    if (currentPos >= 0)
    {
        ui->tabWidget->setCurrentIndex(currentPos);
    }
}

void FileClipboardDialog::onCurrentTabChanged(int tabPos)
{
    if ((_tabsLoaded == false) || (tabPos < 0))
    {
        return;
    }

    int slot = ui->tabWidget->widget(tabPos)->property("slot").toInt();
    QSettings settings;
    settings.setValue(CB_FILE_CLIPBOARD_CURRENT_SLOT, slot);
}

void FileClipboardDialog::migrateLegacyClipboard()
{
    // The original clipboard stored a single flat 256-entry array. Fold it into
    // slot 0 so existing snippets survive the upgrade to tabs.
    QSettings settings;
    settings.beginReadArray(CB_FILE_CLIBBOARD_SETTINGS_ROOT);
    QStringList legacy;
    for (int row = 0; row < kRowCount; row++)
    {
        settings.setArrayIndex(row);
        legacy.append(settings.value("Text").toString());
    }
    settings.endArray();

    settings.beginWriteArray(slotEntriesArray(0));
    for (int row = 0; row < kRowCount; row++)
    {
        settings.setArrayIndex(row);
        settings.setValue("Text", legacy.at(row));
    }
    settings.endArray();

    settings.setValue(slotNameKey(0), tr("Clipboard"));
    settings.setValue(CB_FILE_CLIPBOARD_OPEN_SLOTS, QString("0"));
}

void FileClipboardDialog::onAddTabClicked()
{
    int slot = lowestFreeSlot();
    QTableWidget* table = addTabForSlot(slot);
    ui->tabWidget->setCurrentWidget(table);
    saveOpenSlots();
}

void FileClipboardDialog::onTabCloseRequested(int tabPos)
{
    // Never close the last open tab; closing only hides the slot, the entries
    // remain in settings for when the slot is reopened.
    if (ui->tabWidget->count() <= 1)
    {
        return;
    }

    QWidget* widget = ui->tabWidget->widget(tabPos);
    ui->tabWidget->removeTab(tabPos);
    delete widget;
    saveOpenSlots();
}

int FileClipboardDialog::tabPosForSlot(int slot) const
{
    int pos = -1;
    for (int i = 0; i < ui->tabWidget->count(); i++)
    {
        if (ui->tabWidget->widget(i)->property("slot").toInt() == slot)
        {
            pos = i;
            break;
        }
    }
    return pos;
}

void FileClipboardDialog::onTabDoubleClicked(int tabPos)
{
    if (tabPos < 0)
    {
        return;
    }

    cancelTabRename();

    QTabBar* tabBar = ui->tabWidget->tabBar();
    _renameSlot = ui->tabWidget->widget(tabPos)->property("slot").toInt();
    _renameEditor = new QLineEdit(tabBar);
    _renameEditor->setText(ui->tabWidget->tabText(tabPos));
    _renameEditor->setGeometry(tabBar->tabRect(tabPos));
    _renameEditor->selectAll();
    connect(_renameEditor, &QLineEdit::editingFinished, this, &FileClipboardDialog::commitTabRename);
    _renameEditor->show();
    _renameEditor->setFocus();

    // Watch the whole application so a click anywhere outside the editor (even
    // on non-focusable areas that wouldn't trigger editingFinished) commits.
    qApp->installEventFilter(this);
}

void FileClipboardDialog::commitTabRename()
{
    if (_renameEditor == nullptr)
    {
        return;
    }

    // Clear the editor pointer first; editingFinished can fire again (focus-out
    // after the commit) and must be a no-op.
    QLineEdit* editor = _renameEditor;
    _renameEditor = nullptr;
    qApp->removeEventFilter(this);

    QString name = editor->text().trimmed();
    int tabPos = tabPosForSlot(_renameSlot);
    if ((tabPos >= 0) && (name.isEmpty() == false))
    {
        ui->tabWidget->setTabText(tabPos, name);
        QSettings settings;
        settings.setValue(slotNameKey(_renameSlot), name);
    }
    editor->deleteLater();
}

void FileClipboardDialog::cancelTabRename()
{
    if (_renameEditor != nullptr)
    {
        QLineEdit* editor = _renameEditor;
        _renameEditor = nullptr;
        qApp->removeEventFilter(this);
        editor->deleteLater();
    }
}

void FileClipboardDialog::sendItemTriggered(QTableWidget* table, int index)
{
    QTableWidgetItem* item = table->item(index, 0);
    if ((item != nullptr) && (item->text().length() > 0))
    {
        ChildForm* c = MainWindow::getMainWindow()->getActiveChildWindow();
        if (c != nullptr)
        {
            QString text = item->text();
            if (ui->returnCheckBox->isChecked())
            {
                text += "\r";
            }
            if (ui->newLineCheckBox->isChecked())
            {
                text += "\n";
            }

            c->sendText(text);
            c->setTrackOutput(true);
            c->setFocus();
            c->activateWindow();
        }
    }
}

void FileClipboardDialog::onTableCellChanged(int row, int column)
{
    UNREF_PARAM(column);
    if (_tabsLoaded == false)
    {
        return;
    }

    QTableWidget* table = qobject_cast<QTableWidget*>(sender());
    if (table == nullptr)
    {
        return;
    }

    int slot = table->property("slot").toInt();
    QTableWidgetItem* item = table->item(row, 0);
    QString text = (item != nullptr) ? item->text() : QString();

    QSettings settings;
    settings.beginWriteArray(slotEntriesArray(slot));
    settings.setArrayIndex(row);
    settings.setValue("Text", text);
    settings.endArray();
}

void FileClipboardDialog::on_newLineCheckBox_toggled(bool checked)
{
    QSettings settings;
    settings.setValue(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine", checked);
}

void FileClipboardDialog::on_returnCheckBox_toggled(bool checked)
{
    QSettings settings;
    settings.setValue(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendReturn", checked);
}

void FileClipboardDialog::loadNewLineSettings()
{
    QSettings settings;
    bool sendNewLineChecked = settings.value(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendNewLine", true).toBool();
    ui->newLineCheckBox->setChecked(sendNewLineChecked);
    bool sendReturnChecked = settings.value(CB_FILE_CLIBBOARD_SETTINGS_ROOT "SendReturn", true).toBool();
    ui->returnCheckBox->setChecked(sendReturnChecked);
}

void FileClipboardDialog::on_sendButton_clicked()
{
    if (ui->searchComboBox->hasFocus() == true)
    {
        on_searchButton_clicked();
    }
    else
    {
        QTableWidget* table = currentTable();
        if (table != nullptr)
        {
            sendItemTriggered(table, table->currentRow());
        }
    }
}

void FileClipboardDialog::on_searchButton_clicked()
{
    QTableWidget* table = currentTable();
    if (table == nullptr)
    {
        return;
    }

    QString searchText = ui->searchComboBox->currentText();
    if (searchText.length() > 0)
    {
        int numRows = table->rowCount();
        int index = table->currentRow() + 1;
        for (int row = 0; row < numRows; row++)
        {
            QTableWidgetItem* item = table->item(index, 0);
            if ((item != nullptr) && (item->text().length() > 0) && (item->text().contains(searchText)))
            {
                table->setCurrentItem(table->item(index, 0));
                table->setFocus();
                break;
            }
            index = (index + 1) % numRows;
        }
    }
}

void FileClipboardDialog::keyPressEvent(QKeyEvent* e)
{
    if (e->key() != Qt::Key_Escape)
    {
        QWidget::keyPressEvent(e);
    }
}

bool FileClipboardDialog::eventFilter(QObject* obj, QEvent* event)
{
    bool ret = true;
    if ((event->type() == QEvent::MouseButtonPress) && (_renameEditor != nullptr))
    {
        // A press anywhere that isn't the editor itself commits the rename. The
        // press is not consumed, so the click still does its normal job (e.g.
        // switching tabs).
        QWidget* pressed = qobject_cast<QWidget*>(obj);
        if ((pressed != _renameEditor) && (_renameEditor->isAncestorOf(pressed) == false))
        {
            commitTabRename();
        }
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (obj == _renameEditor)
        {
            // Escape cancels the in-place rename; Enter falls through to the
            // QLineEdit, which commits via its editingFinished signal.
            if (keyEvent->key() == Qt::Key_Escape)
            {
                ret = false;
                cancelTabRename();
            }
        }
        else if (obj == ui->searchComboBox)
        {
            if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return))
            {
                ret = false;
                on_searchButton_clicked();
            }
        }
    }
    if (ret == true)
    {
        ret = QWidget::eventFilter(obj, event);
    }
    return ret;
}
