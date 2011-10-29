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



Zdispatch::Zdispatch()
{}

Zdispatch::~Zdispatch()
{}


Zdispatch::Zdispatch(QString a_host, QString a_port)
{
    std::cout << "Zdispatch::Zdispatch constructeur" << std::endl;
    m_context = new zmq::context_t(1);
    m_host = a_host;
    m_port = a_port;
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
        zmq::message_t message;
        receiver.recv(&message);

        std::cout << "Received request: [" << (char*) message.data() << "]" << std::endl;

        bo l_payload = bo((char*)message.data());

        std::cout << "l_payload : " << l_payload << std::endl;

        emit payload(l_payload);
    }

}




Zeromq::Zeromq()
{}


Zeromq::~Zeromq()
{}

Zeromq::Zeromq(QString m_host, QString m_port)
{
    qDebug() << "Zeromq::construct";

    QThread *thread_dispatch = new QThread;
    dispatch = new Zdispatch(m_host, m_port);
    connect(thread_dispatch, SIGNAL(started()), dispatch, SLOT(receive_payload()));
    dispatch->moveToThread(thread_dispatch);
    thread_dispatch->start();

}
