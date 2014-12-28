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
#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#ifndef Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif
#include <QNetworkReply>

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    static boost::shared_ptr<UpdateChecker> instance();
    void checkForNewVersion();
    int32_t getLatestVersion();
    QString getLatestVersionStr();
    ~UpdateChecker();

private slots:
    void slotError(QNetworkReply::NetworkError);
    void slotReadyRead();
    void replyFinished(QNetworkReply* reply);

signals:
    void newVersionAvailable();

private:
    UpdateChecker();

    static boost::shared_ptr<UpdateChecker> _inst;
    QNetworkReply* _reply;
    QString _result;
    int32_t _latestVersion;
    QString _latestVersionStr;
    boost::scoped_ptr<QNetworkAccessManager> _manager;
};

#endif // UPDATECHECKER_H
