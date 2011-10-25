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


#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <QDebug>
#include <QDomDocument>
#include <QxtCore/QxtCommandOptions>

#include "libmemcached/memcached.hpp"

#include "nosql.h"
#include "zeromq.h"
#include "stats_process.h"
#include "stats_cpu.h"
#include "stats_load.h"
#include "stats_uptime.h"
#include "stats_memory.h"
#include "stats_network.h"


enum WorkerType {
    WCPU=1,
    WMEMORY=2,
    WNETWORK=3,
    WLOAD=4,
    WUPTIME=5,
    WPROCESS=6
};

typedef QMap<QString, WorkerType> StringToEnumMap;


class NodecastMemcache
{
public:
    NodecastMemcache();
    ~NodecastMemcache();

    memcached_server_st *servers;
    memcached_st *memc;
    memcached_return rc;

    memcached_server_st *memcached_servers_parse (char *server_strings);
};



class Worker : public QObject
{
    Q_OBJECT
public:
    Worker();
    ~Worker();
    void Init(QString worker_type, QString memcached_keycache);
    NodecastMemcache nodecast_memcache;
    Nosql *nosql;
    //Amqp *amqp;
    Zeromq *zeromq;
    //AmqpListener *listener;
    Stats *stats_worker;

    // params



public slots:
    void s_delete_cache(QString pub_uuid);
};




#endif // MAIN_H
