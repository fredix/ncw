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
#include <signal.h>

#include <QDebug>
#include <QDomDocument>
#include <QxtCore/QxtCommandOptions>

#include "libmemcached/memcached.hpp"

#include "nosql.h"
#include "zeromq.h"
#include "dispatcher.h"
#include "service.h"
#include "process.h"
#include "stats_process.h"
#include "stats_cpu.h"
#include "stats_load.h"
#include "stats_uptime.h"
#include "stats_memory.h"
#include "stats_network.h"
#include "get_payload.h"


enum WorkerType {
    WSERVICE=1,
    WDISPATCHER=2,
    WPROCESS=3,
    GK_CPU=4,
    GK_MEMORY=5,
    GK_NETWORK=6,
    GK_LOAD=7,
    GK_UPTIME=8,
    GK_PROCESSUS=9,
    GK_FILESYSTEM=10,
    GK_PAYLOAD=11
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



class Zworker : public QObject
{
    Q_OBJECT
public:
    Zworker();
    ~Zworker();
    void Init(QString worker_type, QString worker_name, QString memcached_keycache, QString child_exec);
    NodecastMemcache nodecast_memcache;
    Nosql *nosql;
    Zeromq *zeromq;
    Worker *worker;
    Process *process;
    Service *service;

    // params



public slots:
    void s_delete_cache(QString pub_uuid);
};


#endif // MAIN_H
