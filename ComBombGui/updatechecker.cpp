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
#include "updatechecker.h"
#include "versioning.h"
#include "../unparam.h"
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

boost::shared_ptr<UpdateChecker> UpdateChecker::_inst;

UpdateChecker::UpdateChecker()
    : _reply(NULL),
    _latestVersion(-1),
    _manager(new QNetworkAccessManager(this))
{
}

UpdateChecker::~UpdateChecker()
{
    _manager->disconnect();
    _reply->disconnect();
}

boost::shared_ptr<UpdateChecker> UpdateChecker::instance()
{
    if (_inst == NULL)
    {
        _inst.reset(new UpdateChecker());
    }
    return _inst;
}

void UpdateChecker::checkForNewVersion()
{
    connect(_manager.get(), SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    _reply = _manager->get(QNetworkRequest(QUrl("https://api.github.com/repos/cdesjardins/ComBomb/tags")));

    connect(_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
    connect(_reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError)));
}

void UpdateChecker::slotError(QNetworkReply::NetworkError err)
{
#ifdef QT_DEBUG
    qDebug("Error: " + err);
#else
    UNREF_PARAM(err);
#endif
}

void UpdateChecker::replyFinished(QNetworkReply*)
{
    QJsonDocument d(QJsonDocument::fromJson(_result.toUtf8()));
    QJsonArray tags = d.array();
    QString verStr = tags[0].toObject()["name"].toString();
    int32_t currentVersion = parseVersionStr(getVersion());
    _latestVersion = parseVersionStr(verStr.toLocal8Bit().constData());
    if ((_latestVersion != -1) && (currentVersion != -1) && (_latestVersion > currentVersion))
    {
        _latestVersionStr = verStr;
        emit newVersionAvailable();
    }
}

void UpdateChecker::slotReadyRead()
{
    if (_reply != NULL)
    {
        QByteArray result = _reply->readAll();
        _result += result.constData();
    }
}

int32_t UpdateChecker::getLatestVersion()
{
    return _latestVersion;
}

QString UpdateChecker::getLatestVersionStr()
{
    return _latestVersionStr;
}

