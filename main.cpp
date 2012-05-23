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

/*
static int s_interrupted = 0;
static void s_signal_handler (int signal_value)
{
    s_interrupted = 1;
}

static void s_catch_signals (void)
{
    struct sigaction action;
    //action.sa_handler = s_signal_handler;
    action.sa_handler = Zeromq::hupSignalHandler;

    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}
*/

// http://doc.qt.nokia.com/4.7/unix-signals.html
static void setup_unix_signal_handlers()
{
    struct sigaction hup, term;

    hup.sa_handler = Zeromq::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    /*if (sigaction(SIGHUP, &hup, 0) > 0)
       return 1;*/

    term.sa_handler = Zeromq::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    /*if (sigaction(SIGTERM, &term, 0) > 0)
       return 2;

    return 0;*/

    sigaction (SIGINT, &hup, NULL);
    sigaction (SIGTERM, &term, NULL);
}

StringToEnumMap enumToWorker;



Zworker::Zworker()
{
    qDebug() << "Zworker construct";
    worker = NULL;
    service = NULL;
}

Zworker::~Zworker()
{
    qDebug() << "DELETE ZWORKER !!!!!";
    delete(this->nosql);
    delete(this->zeromq);
}


void Zworker::s_delete_cache(QString cache)
{
    qDebug() << "Zworker::s_delete_cache";

    nodecast_memcache.rc = memcached_delete(nodecast_memcache.memc, cache.toStdString().c_str(), strlen(cache.toStdString().c_str()), (time_t)0);

    if (nodecast_memcache.rc == MEMCACHED_SUCCESS)
        qDebug() <<  "Worker::s_delete_cache => cache DELETED : " << cache;
    else
        qDebug() << "Worker::s_delete_cache => Couldn't delete cache: " << memcached_strerror(nodecast_memcache.memc, nodecast_memcache.rc);
}


void Zworker::Init(QString worker_type, QString worker_name, QString memcached_keycache, QString child_exec)
{

    qDebug() << worker_type;
    qRegisterMetaType<bson::bo>("bson::bo");


    switch (enumToWorker[worker_type])
    {   
    case WSERVICE:
        qDebug() << "WSERVICE : " << worker_type ;
        zeromq->payloader();
        service = new Service(*this->nosql);
        //this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;


    case WDISPATCHER:
        qDebug() << "WDISPATCHER : " << worker_type ;
        worker = new Dispatcher(*this->nosql);

        zeromq->dispatcher ();
        this->connect(zeromq->dispatch_http, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        this->connect(zeromq->dispatch_xmpp, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        this->connect(worker, SIGNAL(return_payload(bson::bo)), zeromq->dispatch_http, SLOT(push_payload(bson::bo)), Qt::DirectConnection);
        break;

    case WPROCESS:
        qDebug() << "WPROCESS : " << worker_type ;
        zeromq->payloader();
        process = new Process(*this->nosql);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), process, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;


    case GK_CPU:
        qDebug() << "WCPU : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_cpu(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_LOAD:
        qDebug() << "WLOAD : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_load(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_MEMORY:
        qDebug() << "WMEMORY : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_memory(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_NETWORK:
        qDebug() << "WNETWORK : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_network(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_UPTIME:
        qDebug() << "WUPTIME : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_uptime(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_PROCESSUS:
        qDebug() << "WPROCESS : " << worker_type ;
        zeromq->payloader();
        worker = new Stats_process(*this->nosql, memcached_keycache);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case GK_PAYLOAD:
        qDebug() << "WPAYLOAD : " << worker_type ;
        zeromq->payloader();
        worker = new Get_payload(*this->nosql);
        this->connect(zeromq->payload, SIGNAL(payload(bson::bo)), worker, SLOT(s_job_receive(bson::bo)), Qt::BlockingQueuedConnection);
        break;


    default:
        qDebug() << "worker unknown : " << worker_type ;
        delete(this);
        exit(1);
    }

    if (worker)
    {
        this->connect(worker, SIGNAL(return_tracker(bson::bo)), zeromq->tracker, SLOT(push_tracker(bson::bo)));
        this->connect(worker, SIGNAL(delete_cache(QString)), this, SLOT(s_delete_cache(QString)));
        worker->init("");
    }
    else if (process)
    {
        this->connect(process, SIGNAL(return_tracker(bson::bo)), zeromq->tracker, SLOT(push_tracker(bson::bo)));
        process->init(child_exec, worker_name);
    }
    else if (service)
    {
        this->connect(service, SIGNAL(return_tracker(bson::bo)), zeromq->tracker, SLOT(push_tracker(bson::bo)));
        service->init(child_exec, worker_name);
    }

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
    QString worker_name;
    QString memcached_ip;
    QString memcached_keycache;
    int memcached_port;
    QString mongodb_ip;
    QString mongodb_base;
    QString zeromq_ip;
    QString zeromq_port;
    QString child_exec;



    enumToWorker.insert(QString("dispatcher"), WDISPATCHER);
    enumToWorker.insert(QString("service"), WSERVICE);
    enumToWorker.insert(QString("process"), WPROCESS);
    enumToWorker.insert(QString("gk_cpu"), GK_CPU);
    enumToWorker.insert(QString("gk_memory"), GK_MEMORY);
    enumToWorker.insert(QString("gk_network"), GK_NETWORK);
    enumToWorker.insert(QString("gk_load"), GK_LOAD);
    enumToWorker.insert(QString("gk_uptime"), GK_UPTIME);
    enumToWorker.insert(QString("gk_processus"), GK_PROCESSUS);
    enumToWorker.insert(QString("gk_filesystem"), GK_FILESYSTEM);
    enumToWorker.insert(QString("gk_payload"), GK_PAYLOAD);




    QCoreApplication nodecast_worker(argc, argv);


    QxtCommandOptions options;
    options.add("debug", "show debug informations");
    options.alias("debug", "d");
    options.add("worker-type", "set the worker type (dispatcher|service|process|cpu|memory|network|load|uptime|processus)", QxtCommandOptions::Required);
    options.alias("worker-type", "wt");

    options.add("worker-name", "set the worker name", QxtCommandOptions::Optional);
    options.alias("worker-name", "wn");

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

    options.add("exec", "set the exec program to launch", QxtCommandOptions::Optional);
    options.alias("exec", "ex");


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

    if(options.count("worker-name")) {
        worker_name = options.value("worker-name").toString();
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


    if(options.count("exec")) {
        child_exec = options.value("exec").toString();
    }


    //  s_catch_signals ();

/*
    QThread *thread_interrupt = new QThread;
    CatchInterrupt *signal_catch = new CatchInterrupt();
    QObject::connect(thread_interrupt, SIGNAL(started()), signal_catch, SLOT(init()));
    signal_catch->moveToThread(thread_interrupt);
    thread_interrupt->start();
*/

    setup_unix_signal_handlers();

    Zworker *zworker = new Zworker;
    zworker->nodecast_memcache.memc= memcached_create(NULL);
    zworker->nodecast_memcache.servers= memcached_server_list_append(zworker->nodecast_memcache.servers, memcached_ip.toAscii(), memcached_port, &zworker->nodecast_memcache.rc);
    zworker->nodecast_memcache.rc= memcached_server_push(zworker->nodecast_memcache.memc, zworker->nodecast_memcache.servers);

    if (zworker->nodecast_memcache.rc == MEMCACHED_SUCCESS)
      qDebug() << "Added memcached server successfully";
    else
      qDebug() << "Couldn't add memcached server: " << memcached_strerror(zworker->nodecast_memcache.memc, zworker->nodecast_memcache.rc);


    zworker->nosql = new Nosql(mongodb_ip, mongodb_base);
    zworker->zeromq = new Zeromq(zeromq_ip, zeromq_port);
    zworker->Init(worker_type, worker_name, memcached_keycache, child_exec);


    qDebug() << "end";



    return nodecast_worker.exec();
}



