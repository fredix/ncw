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

#include <QFile>
#include <QObject>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QCoreApplication>
#include <QSocketNotifier>
#include <QQueue>

#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <zmq.hpp>

#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"

#include "service.h"
#include "process.h"
#include "ncw_global.h"

using namespace mongo;
using namespace bson;


enum WorkerType {
    WSERVICE=1,
    WPROCESS=2
};
typedef QMap<QString, WorkerType> StringToEnumMap;
typedef QSharedPointer<QThread> Zthread_pushPtr;


class Zstream : public QObject
{
    Q_OBJECT
public:
    Zstream(zmq::context_t *a_context, QString a_host, QString a_directory);
    ~Zstream();
    void get_stream(bson::bo payload, string filename, bool *status);


private:
    QSocketNotifier *check_stream;
    zmq::context_t *m_context;
    zmq::socket_t *z_receive;
    zmq::message_t *z_message;
    QMutex *m_mutex;
    QFile *data_stream;

    QString m_host;
    QString m_directory;
    QString m_uuid;

private slots:
    void stream_payload();
    void stream_payload2();
};


class Ztracker : public QObject
{
    Q_OBJECT
public:
    Ztracker(zmq::context_t *a_context, QString a_host, QString a_port);
    ~Ztracker();
    QString m_worker_port;

private:
    zmq::context_t *m_context;
    zmq::socket_t *z_sender;
    zmq::message_t *z_message;
    QMutex *m_mutex;

    QString m_host;
    QString m_port;
    QString m_uuid;

signals:
    void worker_port(QString worker_port, QString worker_uuid);

public slots:
    void init();
    void push_tracker(bson::bo tracker);
};


class Zpayload : public QObject
{
    Q_OBJECT
public:
    Zpayload(zmq::context_t *a_context, ncw_params ncw, QString a_ncs_ip);
    ~Zpayload();


private:
    QSocketNotifier *check_receive_payload;
    QSocketNotifier *check_pubsub_payload;
    zmq::socket_t *m_socket_worker;
    zmq::socket_t *m_socket_pubsub;
    zmq::message_t *m_message;
    zmq::message_t *m_pubsub_message;
    zmq_pollitem_t m_items [2];

    zmq::context_t *m_context;
    zmq::socket_t *m_receiver;

    QString m_host;
    QString m_port;
    QString m_worker_name;
    QString m_uuid;
    QString m_node_uuid;
    QString m_node_password;
    ncw_params m_ncw;

signals:    
    void emit_payload(bson::bo data);
    void emit_pubsub(QString data);
    void emit_launch_worker(ncw_params);

public slots:    
    void init_payload(QString worker_port, QString worker_uuid);
    void receive_payload();
    void push_payload(bson::bo data);
    void pubsub_payload();
};




class Zeromq : public QObject
{
    Q_OBJECT
public:
    Zeromq(zmq::context_t *a_context, ncw_params ncw, QString ncs_ip);
    ~Zeromq();
    static bool lock_push_payload();
    static void unlock_push_payload();
    static bool check_push_payload;
    static QMutex mutex_push_payload;

    static bool lock_get_stream();
    static void unlock_get_stream();
    static bool check_get_stream;
    static QMutex mutex_get_stream;


    Ztracker *tracker;
    Zpayload *payload;
    Zstream *zstream;


private:
  /*  QHash<QString, Ztracker_pushPtr> ztracker_push;
    QHash<QString, Zpayload_pushPtr> zpayload_push;
    QHash<QString, Zstream_pushPtr> zstream_push;

    QHash<QString, Zthread_pushPtr> q_thread_tracker;
    QHash<QString, Zthread_pushPtr> q_thread_payload;
    QHash<QString, Zthread_pushPtr> q_thread_stream;
*/
    //QThread **thread_tracker;
    //QThread **thread_payload;
    //QThread **thread_stream;

    QThread *thread_tracker;
    QThread *thread_payload;
    QThread *thread_stream;

    //Zdispatcher *ncw_dispatcher;

    Process *ncw_process;
    Service *ncw_service;

    QMutex *m_port_mutex;
    zmq::context_t *m_context;
    ncw_params m_ncw;
    QString m_ncs_ip;
};


typedef QSharedPointer<Zeromq> Zeromq_pushPtr;
typedef QSharedPointer<Ztracker> Ztracker_pushPtr;
typedef QSharedPointer<Zpayload> Zpayload_pushPtr;
typedef QSharedPointer<Zstream> Zstream_pushPtr;


class Zdispatcher : public QObject
{
    Q_OBJECT
public:
    Zdispatcher(ncw_params ncw, QString ncs_ips);
    ~Zdispatcher();
    static Zdispatcher *getInstance();

    QHash<int, Zpayload_pushPtr> zpayload;
    QHash<int, Zstream_pushPtr> zstream;

private:
    QThread *thread_service;
    zmq::context_t *z_context;
    QHash<int, Zeromq_pushPtr> zeromq_push;

    int ncs_counter, ncs_number;
    static Zdispatcher *_singleton;
    Ztracker *tracker;
    QThread *thread_tracker;
    QThread *thread_payload;
    QThread *thread_stream;

    Process *ncw_process;
    Service *ncw_service;



    QMutex *m_mutex;

    QString m_host;
    QString m_port;
    QString m_uuid;


public slots:
    void push_payload(bson::bo data);
    void push_stream(bson::bo payload, string filename, bool *status);
};


#endif // ZEROMQ_H
