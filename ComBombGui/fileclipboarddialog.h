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
#ifndef FILECLIPBOARDDIALOG_H
#define FILECLIPBOARDDIALOG_H

#include "cbdialog.h"
#include "fileclipboardheader.h"

class QTableWidget;
class QLineEdit;

namespace Ui {
class FileClipboardDialog;
}

class FileClipboardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FileClipboardDialog(QWidget* parent = 0);
    virtual ~FileClipboardDialog();

protected:
    void loadNewLineSettings();
    void keyPressEvent(QKeyEvent* e);
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void sendItemTriggered(QTableWidget* table, int index);
    void onTableCellChanged(int row, int column);
    void onAddTabClicked();
    void onTabCloseRequested(int tabPos);
    void onTabDoubleClicked(int tabPos);
    void onCurrentTabChanged(int tabPos);
    void commitTabRename();
    void on_newLineCheckBox_toggled(bool checked);
    void on_returnCheckBox_toggled(bool checked);
    void on_sendButton_clicked();
    void on_searchButton_clicked();

private:
    // Tabs are addressed by a stable "slot" number, not by their position in
    // the tab bar or their name. Each slot keeps its name and 256 entries in
    // QSettings; closing a tab only hides the slot, so reopening it restores
    // its contents.
    QTableWidget* createTableForSlot(int slot);
    QTableWidget* addTabForSlot(int slot);
    QTableWidget* currentTable() const;
    QList<int> openSlots() const;
    int lowestFreeSlot() const;
    int tabPosForSlot(int slot) const;
    void loadTabs();
    void migrateLegacyClipboard();
    void saveOpenSlots();
    void cancelTabRename();

    Ui::FileClipboardDialog* ui;
    bool _tabsLoaded;

    // In-place tab rename: a QLineEdit overlaid on the tab bar. _renameSlot is
    // the (stable) slot being edited, so a commit targets the right tab even if
    // its position changes.
    QLineEdit* _renameEditor;
    int _renameSlot;

    static const int kRowCount = 256;
};

#endif // FILECLIPBOARDDIALOG_H
