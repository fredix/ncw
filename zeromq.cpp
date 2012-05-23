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

#include "zeromq.h"



int Zeromq::sighupFd[2]={};
int Zeromq::sigtermFd[2]={};

Ztracker::Ztracker(zmq::context_t *a_context, QString a_host, QString a_port) : m_context(a_context)
{
    m_mutex = new QMutex();

    m_host = a_host;
    m_port = a_port;

    // Prepare our context and socket        
    z_message = new zmq::message_t(2);
    z_sender = new zmq::socket_t (*m_context, ZMQ_REQ);

    uint64_t hwm = 5000;
    zmq_setsockopt (z_sender, ZMQ_HWM, &hwm, sizeof (hwm));
}


void Ztracker::init()
{
    std::cout << "Connecting to the ncs tracker" << std::endl;
    z_sender->connect ("tcp://localhost:5569");

    // Do 10 requests, waiting each time for a response
    //for (int request_nbr = 0; request_nbr != 2; request_nbr++) {

    bo ping = BSON("type" << "init" << "action" << "ping");

    z_message->rebuild(ping.objsize());
    memcpy ((void *) z_message->data (), (char*)ping.objdata(), ping.objsize());
  //  std::cout << "Sending Hello " << request_nbr << "…" << std::endl;
    z_sender->send (*z_message);

    // Get the reply.
    zmq::message_t reply;
    z_sender->recv (&reply);
    std::cout << "Received : " << (char*) reply.data() << std::endl;

      /*  if (s_interrupted) {
                    printf ("W: interrupt received, killing server…\n");
                    qApp->exit();
                }*/

    //}
}

Ztracker::~Ztracker()
{
    std::cout << "Ztracker::~Ztracker END\r\n" << std::endl;
}



void Ztracker::push_tracker(bson::bo payload)
{
    m_mutex->lock();
    std::cout << "Ztracker::push_tracker\r\n" << std::endl;

    std::cout << "TRACKER : " << payload << std::endl;


    bo l_payload;

    if (payload["action"].str() != "register")
    {
        l_payload = BSON("payload" << payload << "uuid" << m_uuid.toStdString());

        std::cout << "PAYLOAD ADDED FIELD : " << l_payload << std::endl;
    }
    else
    {
        qDebug() << "REGISTER WORKER";
        l_payload = BSON("payload" << payload);
    }

    /****** PUSH API PAYLOAD *******/
    z_message->rebuild(l_payload.objsize());
    memcpy(z_message->data(), (char*)l_payload.objdata(), l_payload.objsize());
    z_sender->send(*z_message);
    /************************/

    // Get the reply.
    zmq::message_t reply;
    z_sender->recv (&reply);

    bo r_payload = bo((char*)reply.data());

    std::cout << "Received : " << r_payload << std::endl;

    if (r_payload.hasField("uuid"))
    {
        std::cout << "UUID : " << r_payload["uuid"] << std::endl;
        m_uuid = QString::fromStdString(r_payload["uuid"].str());
    }
    else
    {
        std::cout << "STATUS : " << r_payload["status"] << std::endl;
    }


    m_mutex->unlock();
}


Zpayload::~Zpayload()
{}


Zpayload::Zpayload(zmq::context_t *a_context, QString a_host, QString a_port) : m_context(a_context), m_host(a_host), m_port(a_port)
{
    std::cout << "Zpayload::Zpayload construct" << std::endl;
}

void Zpayload::receive_payload()
{
    QString connection_string = "tcp://" + m_host + ":" + m_port;

    qDebug() << "connection_string : " << connection_string;

    //  Socket to receive messages on
    zmq::socket_t receiver(*m_context, ZMQ_PULL);


    uint64_t hwm = 50000;
    zmq_setsockopt (receiver, ZMQ_HWM, &hwm, sizeof (hwm));


    receiver.connect(connection_string.toAscii().data());


    //  Process tasks forever
    while (true) {
        qDebug() << "Zpayload WHILE : " << m_port;
        zmq::message_t message;
        receiver.recv(&message);

        //std::cout << "Received request: [" << (char*) message.data() << "]" << std::endl;

        bo l_payload = bo((char*)message.data());

        emit payload(l_payload);
    }

}






Zdispatch::Zdispatch()
{}

Zdispatch::~Zdispatch()
{}


Zdispatch::Zdispatch(zmq::context_t *a_context, QString a_host, QString a_port) : m_context(a_context)
{
    std::cout << "Zdispatch::Zdispatch constructeur" << std::endl;
    m_host = a_host;
    m_port = a_port;

    z_message = new zmq::message_t(2);
    z_sender = new zmq::socket_t(*m_context, ZMQ_PUSH);
    z_sender->connect("tcp://*:5559");
}

void Zdispatch::receive_payload()
{
    QString connection_string = "tcp://" + m_host + ":" + m_port;

    qDebug() << "connection_string : " << connection_string;

    //  Socket to receive messages on
    zmq::socket_t receiver(*m_context, ZMQ_PULL);
    receiver.connect(connection_string.toAscii().data());


    //  Process tasks forever
    while (true) {
        qDebug() << "ZDISPTACH WHILE : " << m_port;
        zmq::message_t message;
        receiver.recv(&message);

        //std::cout << "Received request: [" << (char*) message.data() << "]" << std::endl;

        bo l_payload = bo((char*)message.data());

        emit payload(l_payload);
    }

}


void Zdispatch::push_payload(bson::bo payload)
{    
    /****** PUSH API PAYLOAD *******/
    qDebug() << "return Zdispatch::return_payload";
    z_message->rebuild(payload.objsize());
    memcpy(z_message->data(), (char*)payload.objdata(), payload.objsize());
    z_sender->send(*z_message, ZMQ_NOBLOCK);
    /************************/
}


Zeromq::~Zeromq()
{
    qDebug() << "Zeromq DELETE !";
}


Zeromq::Zeromq(QString a_host, QString a_port) : m_host(a_host), m_port(a_port)
{
    qDebug() << "Zeromq::construct";

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
        qFatal("Couldn't create HUP socketpair");

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
        qFatal("Couldn't create TERM socketpair");
    snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(snHup, SIGNAL(activated(int)), this, SLOT(handleSigHup()));
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));


    m_context = new zmq::context_t(1);


    QThread *thread_tracker = new QThread;
    tracker = new Ztracker(m_context, m_host, "5569");
    connect(thread_tracker, SIGNAL(started()), tracker, SLOT(init()));
    tracker->moveToThread(thread_tracker);
    thread_tracker->start();
}



void Zeromq::dispatcher()
{
    QThread *thread_dispatch = new QThread;
    dispatch_http = new Zdispatch(m_context, m_host, m_port);
    connect(thread_dispatch, SIGNAL(started()), dispatch_http, SLOT(receive_payload()));
    dispatch_http->moveToThread(thread_dispatch);
    thread_dispatch->start();


    QThread *thread_dispatch_xmpp = new QThread;
    dispatch_xmpp = new Zdispatch(m_context, m_host, "5557");
    connect(thread_dispatch_xmpp, SIGNAL(started()), dispatch_xmpp, SLOT(receive_payload()));
    dispatch_xmpp->moveToThread(thread_dispatch_xmpp);
    thread_dispatch_xmpp->start();
}



void Zeromq::payloader()
{
    QThread *thread_payload = new QThread;
    payload = new Zpayload(m_context, m_host, m_port);
    connect(thread_payload, SIGNAL(started()), payload, SLOT(receive_payload()));
    payload->moveToThread(thread_payload);
    thread_payload->start();
}


void Zeromq::hupSignalHandler(int)
{
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void Zeromq::termSignalHandler(int)
{
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void Zeromq::handleSigTerm()
{
    snTerm->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    std::cout << "Received SIGTERM" << std::endl;
    snTerm->setEnabled(true);
}

void Zeromq::handleSigHup()
{
    snHup->setEnabled(false);
    char tmp;
    ::read(sighupFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    std::cout << "Received SIGHUP" << std::endl;
    snHup->setEnabled(true);
    qApp->exit();
}
