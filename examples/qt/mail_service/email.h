/****************************************************************************
**   ncw is the nodecast worker, client of the nodecast server
**   Copyright (C) 2010-2013  Frédéric Logier <frederic@logier.org>
**
**   https://github.com/nodecast/ncw
**
**   This program is free software: you can redistribute it and/or modify
**   it under the terms of the GNU Affero General Public License as
**   published by the Free Software Foundation, either version 3 of the
**   License, or (at your option) any later version.
**
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU Affero General Public License for more details.
**
**   You should have received a copy of the GNU Affero General Public License
**   along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#ifndef EMAIL_H
#define EMAIL_H

#include <QObject>
#include <QDebug>
#include <QMutex>
#include <QtNetwork/QHostAddress>
#include <QxtJSON>
//#include <QxtNetwork/qxtsmtp.h>
#include <QxtNetwork>

struct email_params {
    QString smtp_hostname;
    QString smtp_username;
    QString smtp_password;
};

class Email : public QObject
{
    Q_OBJECT
public:
    Email(email_params a_email);
    ~Email();

private:
    QString m_host;
    QString m_username;
    QString m_password;
    QMutex *m_mutex;
    QxtSmtp *m_smtp;
    QxtMailMessage *m_message;

public slots:
    void sendEmail(QVariant json);
    void failed();
    void success();
};

#endif // EMAIL_H
