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


#include <QtCore/QCoreApplication>
#include "main.h"

//namespace po = boost::program_options;

StringToEnumMap enumToWorker;


Worker::Worker()
{
    qDebug() << "Worker construct";
}

Worker::~Worker()
{}


void Worker::s_delete_cache(QString cache)
{
    qDebug() << "Worker::s_delete_cache";

    nodecast_memcache.rc = memcached_delete(nodecast_memcache.memc, cache.toStdString().c_str(), strlen(cache.toStdString().c_str()), (time_t)0);

    if (nodecast_memcache.rc == MEMCACHED_SUCCESS)
        qDebug() <<  "Worker::s_delete_cache => cache DELETED : " << cache;
    else
        qDebug() << "Worker::s_delete_cache => Couldn't delete cache: " << memcached_strerror(nodecast_memcache.memc, nodecast_memcache.rc);
}


void Worker::Init(QString worker_type, QString memcached_keycache)
{

    qDebug() << worker_type;

    switch (enumToWorker[worker_type])
    {
    case WCPU:
        qDebug() << "WCPU : " << worker_type ;
        stats_worker = new Stats_cpu(*this->nosql, memcached_keycache);
        break;

    case WLOAD:
        stats_worker = new Stats_load(*this->nosql, memcached_keycache);
        break;

    case WMEMORY:
        stats_worker = new Stats_memory(*this->nosql, memcached_keycache);
        break;

    case WNETWORK:
        stats_worker = new Stats_network(*this->nosql, memcached_keycache);
        break;

    case WUPTIME:
        stats_worker = new Stats_uptime(*this->nosql, memcached_keycache);
        break;

    case WPROCESS:
        qDebug() << "WPROCESS : " << worker_type ;
        stats_worker = new Stats_process(*this->nosql, memcached_keycache);
        break;

    default:
        qDebug() << "default error : " << worker_type ;
        exit(1);
    }

    qRegisterMetaType<bson::bo>("bson::bo");

    this->connect(zeromq->dispatch, SIGNAL(payload(bson::bo)), stats_worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
    this->connect(stats_worker, SIGNAL(delete_cache(QString)), this, SLOT(s_delete_cache(QString)));
}

NodecastMemcache::NodecastMemcache() {
    this->servers = NULL;        
    qDebug() << "Memcache construct";
}

NodecastMemcache::~NodecastMemcache()
{}





/*
 * nodecast-worker --memcached-ip=127.0.0.1 --memcached-port=11211
 *   --mongodb-ip 127.0.0.1 --mongodb-base=nodecast_prod --qpid-ip=127.0.0.1
 *   --qpid-port=5672
*/

int main(int argc, char *argv[])
{

    bool debug;
    bool verbose;
    QString worker_type;
    QString memcached_ip;
    QString memcached_keycache;
    int memcached_port;
    QString mongodb_ip;
    QString mongodb_base;
    QString zeromq_ip;
    QString zeromq_port;


    enumToWorker.insert(QString("cpu"), WCPU);
    enumToWorker.insert(QString("memory"), WMEMORY);
    enumToWorker.insert(QString("network"), WNETWORK);
    enumToWorker.insert(QString("load"), WLOAD);
    enumToWorker.insert(QString("uptime"), WUPTIME);
    enumToWorker.insert(QString("process"), WPROCESS);




    QCoreApplication nodecast_worker(argc, argv);


    QxtCommandOptions options;
    options.add("debug", "show debug informations");
    options.alias("debug", "d");
    options.add("worker-type", "set the worker type (cpu|memory|network|load|uptime|process)", QxtCommandOptions::Required);
    options.alias("worker-type", "wt");
    options.add("memcached-ip", "set the memcached ip", QxtCommandOptions::Required);
    options.alias("memcached-ip", "mc");
    options.add("memcached-port", "set the memcached port", QxtCommandOptions::Required);
    options.alias("memcached-port", "mp");
    options.add("memcached-keycache", "set the memcached key's environment (development|production)", QxtCommandOptions::Required);
    options.alias("memcached-keycache", "mk");
    options.add("mongodb-ip", "set the mongodb ip", QxtCommandOptions::Required);
    options.alias("mongodb-ip", "mdip");
    options.add("mongodb-base", "set the mongodb base", QxtCommandOptions::Required);
    options.alias("mongodb-base", "mdp");

    options.add("zeromq-ip", "set the zeromq ip", QxtCommandOptions::Required);
    options.alias("zeromq-ip", "zip");
    options.add("zeromq-port", "set the zeromq port", QxtCommandOptions::Required);
    options.alias("zeromq-port", "zpp");

    options.add("verbose", "show more information about the process; specify twice for more detail", QxtCommandOptions::AllowMultiple);
    options.alias("verbose", "v");
    options.add("help", "show this help text");
    options.alias("help", "h");
    options.parse(QCoreApplication::arguments());
    if(options.count("help") || options.showUnrecognizedWarning()) {
        options.showUsage();
        return -1;
    }
    verbose = options.count("verbose");

    if(options.count("worker-type")) {
        worker_type = options.value("worker-type").toString();
    }
    else {
        std::cout << "nodecast-worker: --worker-type requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("memcached-port")) {
        memcached_port = options.value("memcached-port").toInt();
    }
    else {
        std::cout << "nodecast-worker: --memcached-port requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }

    if(options.count("mongodb-base")) {
        mongodb_base = options.value("mongodb-base").toString();
    }
    else {
        std::cout << "nodecast-worker: --mongodb-base requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }




    if(options.count("mongodb-ip")) {
        mongodb_ip = options.value("mongodb-ip").toString();
    }
    else {
        std::cout << "nodecast-worker: --mongodb-ip requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("memcached-ip")) {
        memcached_ip = options.value("memcached-ip").toString();
    }
    else {
        std::cout << "nodecast-worker: --memcached-ip requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }



    if(options.count("memcached-keycache")) {
        memcached_keycache = options.value("memcached-keycache").toString();
    }
    else {
        std::cout << "nodecast-worker: --memcached-keycache requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }




    if(options.count("zeromq-port")) {
        zeromq_port = options.value("zeromq-port").toString();
    }
    else {
        std::cout << "nodecast-dispatcher: --zeromq-port requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("zeromq-ip")) {
        zeromq_ip = options.value("zeromq-ip").toString();
    }
    else {
        std::cout << "nodecast-dispatcher: --zeromq-ip requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    Worker worker;

    worker.nodecast_memcache.memc= memcached_create(NULL);
    worker.nodecast_memcache.servers= memcached_server_list_append(worker.nodecast_memcache.servers, memcached_ip.toAscii(), memcached_port, &worker.nodecast_memcache.rc);
    worker.nodecast_memcache.rc= memcached_server_push(worker.nodecast_memcache.memc, worker.nodecast_memcache.servers);

    if (worker.nodecast_memcache.rc == MEMCACHED_SUCCESS)
      qDebug() << "Added memcached server successfully";
    else
      qDebug() << "Couldn't add memcached server: " << memcached_strerror(worker.nodecast_memcache.memc, worker.nodecast_memcache.rc);


    worker.nosql = new Nosql(mongodb_ip, mongodb_base);
    worker.zeromq = new Zeromq(zeromq_ip, zeromq_port);

    worker.Init(worker_type, memcached_keycache);


    qDebug() << "end";

    return nodecast_worker.exec();
}



