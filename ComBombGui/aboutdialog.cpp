/*
    ComBomb - Terminal emulator
    Copyright (C) 2014  Chris Desjardins
    http://blog.chrisd.info cjd@chrisd.info

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
#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "versioning.h"
#include "cblabel.h"
#include "mainwindow.h"
#include <boost/date_time/posix_time/posix_time.hpp>

CBLabel::CBLabel(QWidget* parent)
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

void CBLabel::mouseDoubleClickEvent(QMouseEvent*)
{
    if (_qImg == NULL)
    {
        _qImg = new QImage();
        _qImg->load(":/images/myicon.png");
        QPixmap pixma = QPixmap::fromImage(*_qImg);
        setPixmap(pixma);
    }
}

AboutDialog::AboutDialog(QWidget* parent) :
    CBDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    QString ver("ComBomb version: ");
    ver.append(getVersion());
    ui->versionLabel->setText(ver);

    QString uptime("Uptime: ");
    std::stringstream buf;
    const boost::posix_time::time_duration elapsed = MainWindow::getMainWindow()->getStartTimeDelta();
    buf << elapsed;
    uptime.append(buf.str().c_str());
    ui->uptimeLabel->setText(uptime);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

QString AboutDialog::getSettingsRoot()
{
    return objectName();
}

