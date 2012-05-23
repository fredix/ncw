/****************************************************************************
**   ncw is the nodecast worker, client of the nodecast server
**   Copyright (C) 2010-2011  Frédéric Logier <frederic@logier.org>
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

#ifndef ZEROMQ_H
#define ZEROMQ_H


#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <zmq.hpp>
#include <client/gridfs.h>

using namespace mongo;
using namespace bson;


class Ztracker : public QObject
{
    Q_OBJECT
public:
    Ztracker(zmq::context_t *a_context, QString a_host, QString a_port);
    ~Ztracker();


private:
    zmq::context_t *m_context;
    zmq::socket_t *z_sender;
    zmq::message_t *z_message;
    QMutex *m_mutex;

    QString m_host;
    QString m_port;

    QString m_uuid;

public slots:
    void init();
    void push_tracker(bson::bo tracker);
};


class Zpayload : public QObject
{
    Q_OBJECT
public:
    Zpayload(zmq::context_t *a_context, QString a_host, QString a_port);
    ~Zpayload();


private:
    zmq::context_t *m_context;
    QString m_host;
    QString m_port;

signals:
    void payload(bson::bo data);

public slots:
    void receive_payload();
};


class Zdispatch : public QObject
{
    Q_OBJECT
public:
    Zdispatch(zmq::context_t *a_context, QString a_host, QString a_port);
    Zdispatch();
    ~Zdispatch();


private:
    zmq::context_t *m_context;
    zmq::socket_t *z_sender;
    zmq::message_t *z_message;

    QString m_host;
    QString m_port;

signals:
    void payload(bson::bo data);

public slots:
    void receive_payload();
    void push_payload(bson::bo payload);
};


class Zeromq : public QObject
{
    Q_OBJECT
public:
    Zeromq(QString a_host, QString a_port);
    ~Zeromq();
    void dispatcher();
    void payloader();

    Zdispatch *dispatch_http;
    Zdispatch *dispatch_xmpp;
    Ztracker *tracker;
    Zpayload *payload;

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);


public slots:
    // Qt signal handlers.
    void handleSigHup();
    void handleSigTerm();

private:
    zmq::context_t *m_context;
    QString m_host;
    QString m_port;

    static int sighupFd[2];
    static int sigtermFd[2];

    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;
};

#endif // ZEROMQ_H
