/****************************************************************************
**   ncw is the nodecast worker, client of the nodecast server
**   Copyright (C) 2010-2012  Frédéric Logier <frederic@logier.org>
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
    delete(this->zeromq);
}


//void Zworker::Init(QString worker_type, QString worker_name, QString child_exec)
void Zworker::Init(ncw_params ncw)
{
    qRegisterMetaType<bson::bo>("bson::bo");
    qRegisterMetaType<string>("string");


    switch (enumToWorker[ncw.worker_type])
    {   
    case WSERVICE:
        qDebug() << "WSERVICE : " << ncw.worker_type ;
        //zeromq->payloader();
        service = new Service();
        connect(zeromq->payload, SIGNAL(payload(bson::bo)), service, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);
        connect(zeromq->payload, SIGNAL(emit_pubsub(string)), service, SLOT(get_pubsub(string)), Qt::QueuedConnection);

        connect(service, SIGNAL(return_tracker(bson::bo)), zeromq->tracker, SLOT(push_tracker(bson::bo)));
        service->init(ncw);
        connect(service, SIGNAL(push_payload(bson::bo)), zeromq->payload, SLOT(push_payload(bson::bo)));
        connect(service, SIGNAL(get_stream(bson::bo)), zeromq->stream, SLOT(get_stream(bson::bo)), Qt::BlockingQueuedConnection);
        break;

    case WPROCESS:
        qDebug() << "WPROCESS : " << ncw.worker_type ;
        //zeromq->payloader();
        process = new Process();
        connect(zeromq->payload, SIGNAL(payload(bson::bo)), process, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);

        connect(process, SIGNAL(return_tracker(bson::bo)), zeromq->tracker, SLOT(push_tracker(bson::bo)));
        process->init(ncw);
        connect(process, SIGNAL(push_payload(bson::bo)), zeromq->payload, SLOT(push_payload(bson::bo)));
        break;

    default:
        qDebug() << "worker unknown : " << ncw.worker_type ;
        delete(this);
        qApp->exit (1);
    }


}



int main(int argc, char *argv[])
{

    bool debug;
    bool verbose;

    ncw_params ncw;


    enumToWorker.insert(QString("service"), WSERVICE);
    enumToWorker.insert(QString("process"), WPROCESS);

    QCoreApplication nodecast_worker(argc, argv);


    QxtCommandOptions options;
    options.add("debug", "show debug informations");
    options.alias("debug", "d");
    options.add("worker-type", "set the worker type (|service|process|)", QxtCommandOptions::Required);
    options.alias("worker-type", "wt");

    options.add("worker-name", "set the worker name", QxtCommandOptions::Required);
    options.alias("worker-name", "wn");

    options.add("ncs-ip", "set the nodecast ip", QxtCommandOptions::Required);
    options.alias("ncs-ip", "nip");
    options.add("ncs-port", "set the nodecast port", QxtCommandOptions::Required);
    options.alias("ncs-port", "npt");

    options.add("node-uuid", "set the node uuid", QxtCommandOptions::Required);
    options.add("node-password", "set the node password", QxtCommandOptions::Required);


    options.add("exec", "set the exec program to launch", QxtCommandOptions::Required);
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
        ncw.worker_type = options.value("worker-type").toString();
        if (ncw.worker_type != "service" && ncw.worker_type != "process")
        {
            std::cout << "ncw: --worker-type accept only process or service" << std::endl;
            options.showUsage();
            return -1;
        }
    }
    else {
        std::cout << "ncw: --worker-type requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }

    if(options.count("worker-name")) {
        ncw.worker_name = options.value("worker-name").toString();
    }


    if(options.count("ncs-port")) {
        ncw.ncs_port = options.value("ncs-port").toString();
    }
    else {
        std::cout << "ncw: --ncs-port requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("ncs-ip")) {
        ncw.ncs_ip = options.value("ncs-ip").toString();
    }
    else {
        std::cout << "ncw: --ncs-ip requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }

    if(options.count("node-uuid")) {
        ncw.node_uuid = options.value("node-uuid").toString();
    }
    else {
        std::cout << "ncw: --node-uuid requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }

    if(options.count("node-password")) {
        ncw.node_password = options.value("node-password").toString();
    }
    else {
        std::cout << "ncw: --node-password requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("exec")) {
        ncw.child_exec = options.value("exec").toString();
    }
    else {
        std::cout << "ncw: --exec requires a parameter" << std::endl;
        options.showUsage();
        return -1;
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


//    zworker->zeromq = new Zeromq(ncs_ip, ncs_port, worker_name);
//    zworker->Init(worker_type, worker_name, child_exec);

    zworker->zeromq = new Zeromq(ncw);
    zworker->Init(ncw);


    qDebug() << "end";


    return nodecast_worker.exec();
}



