/****************************************************************************
**   nodecast-worker is a bot worker, part of the backend of nodecast.net
**   Copyright (C) 2010-2011  Frédéric Logier <frederic@logier.org>
**
**   http://gitorious.org/nodecast/nodecast-worker
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

#ifndef ZEROMQ_H
#define ZEROMQ_H


#include <QObject>
#include <QThread>
#include <QDebug>
#include <zmq.hpp>
#include <mongodb/client/gridfs.h>

using namespace mongo;
using namespace bson;


class Zdispatch : public QObject
{
    Q_OBJECT
public:
    Zdispatch(QString a_host, QString a_port);
    Zdispatch();
    ~Zdispatch();


private:
    zmq::context_t *m_context;
    zmq::socket_t *sender;
    zmq::message_t *z_message;
    QString m_host;
    QString m_port;

signals:
    void payload(std::string data);

public slots:
    void receive_payload();
};


class Zeromq : public QObject
{
    Q_OBJECT
public:
    Zeromq();
    Zeromq(QString host, QString port);
    ~Zeromq();

    Zdispatch *dispatch;

private:
    QString m_host;
    QString m_port;
};

#endif // ZEROMQ_H
