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
#include "seriallineswidget.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSignalBlocker>

SerialLinesWidget::SerialLinesWidget(QWidget* parent)
    : QWidget(parent),
    _dtrCheckBox(new QCheckBox(tr("DTR"), this)),
    _rtsCheckBox(new QCheckBox(tr("RTS"), this))
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 0, 6, 0);
    layout->addWidget(_dtrCheckBox);
    layout->addWidget(_rtsCheckBox);

    _dtrCheckBox->setToolTip(tr("Assert DTR on the serial port. Untick for boards that wire DTR to a boot or reset pin."));
    _rtsCheckBox->setToolTip(tr("Assert RTS on the serial port. Untick for boards that wire RTS to a reset pin. "
                                "Ignored with hardware flow control."));
    connect(_dtrCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxToggledSlot()));
    connect(_rtsCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkBoxToggledSlot()));
}

SerialLinesWidget::~SerialLinesWidget()
{
}

void SerialLinesWidget::setLines(const bool dtr, const bool rts)
{
    QSignalBlocker dtrBlocker(_dtrCheckBox);
    QSignalBlocker rtsBlocker(_rtsCheckBox);
    _dtrCheckBox->setChecked(dtr);
    _rtsCheckBox->setChecked(rts);
}

bool SerialLinesWidget::dtr() const
{
    return _dtrCheckBox->isChecked();
}

bool SerialLinesWidget::rts() const
{
    return _rtsCheckBox->isChecked();
}

void SerialLinesWidget::checkBoxToggledSlot()
{
    emit linesChanged(dtr(), rts());
}
