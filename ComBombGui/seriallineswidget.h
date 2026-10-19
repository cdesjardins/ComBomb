/*
    ComBomb - Terminal emulator
    Copyright (C) 2014-2026  Chris Desjardins
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
#ifndef SERIALLINESWIDGET_H
#define SERIALLINESWIDGET_H

#include <QWidget>

class QCheckBox;

// DTR and RTS checkboxes for the toolbar, driving the active serial
// connection's output control lines.
class SerialLinesWidget : public QWidget
{
    Q_OBJECT
public:
    SerialLinesWidget(QWidget* parent = 0);
    virtual ~SerialLinesWidget();
    // Show a connection's line state without emitting linesChanged.
    void setLines(const bool dtr, const bool rts);
    bool dtr() const;
    bool rts() const;

signals:
    void linesChanged(bool dtr, bool rts);

private slots:
    void checkBoxToggledSlot();

private:
    QCheckBox* _dtrCheckBox;
    QCheckBox* _rtsCheckBox;
};

#endif // SERIALLINESWIDGET_H
