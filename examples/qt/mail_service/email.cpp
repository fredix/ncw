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

#include "email.h"
#include <iostream>

Email::Email(email_params a_email) : m_host(a_email.smtp_hostname), m_username(a_email.smtp_username), m_password(a_email.smtp_password)
{
    qDebug() << "Email:Email CONSTRUCT";
    m_mutex = new QMutex();

    m_smtp = NULL;
    m_message = NULL;

}


void Email::sendEmail(QVariant json)
{
    //m_mutex->lock();
    //qDebug() << "!!!!!! Email::sendEmail  : " << json;

    if (json.toMap().contains("exp")
            && json.toMap().contains("dest")
            && json.toMap().contains("object")
            && json.toMap().contains("body")
            )
    {
         QString exp = json.toMap()["exp"].toString();
         QString dest = json.toMap()["dest"].toString();
         QString object = json.toMap()["object"].toString();
         QString body = json.toMap()["body"].toString();

         //QString log = exp + " " + dest + " " + field3;

        if (m_smtp) delete(m_smtp);
        if (m_message) delete(m_message);
        m_smtp = NULL;
        m_message = NULL;



        m_smtp = new QxtSmtp();

        connect(m_smtp, SIGNAL(mailFailed(int,int)), this, SLOT(failed()));
        connect(m_smtp, SIGNAL(mailSent(int)), this, SLOT(success()));


        m_message = new QxtMailMessage();

        m_message->setSender(exp.toLatin1());
        m_message->setSubject(object.toLatin1());
        m_message->setBody(body.toLatin1());
        m_message->addRecipient(dest.toLatin1());

        QHash<QString,QString> headers;
        headers.insert("MIME-Version","1.0");
        headers.insert("Content-type","text/html; charset=utf-8");
        headers.insert("from", exp.toLatin1());
        m_message->setExtraHeaders(headers);

        m_smtp->connectToSecureHost(m_host.toAscii());
        m_smtp->setStartTlsDisabled(true);
        m_smtp->setUsername(m_username.toAscii());
        m_smtp->setPassword(m_password.toAscii());


        m_smtp->send(*m_message);
    }

}

void Email::failed()
{
    qDebug() << "Email::failed";

    m_smtp->disconnectFromHost();
    m_mutex->unlock();
}


void Email::success()
{
    qDebug() << "Email::sucess";

    m_smtp->disconnectFromHost();
    //m_mutex->unlock();
}


Email::~Email()
{}
