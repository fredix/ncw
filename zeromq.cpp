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

        QString l_payload = QString::fromAscii((char*)message.data());


        //std::cout << "BEFORE BO" << std::endl;
        //bo data = mongo::fromjson((char*)message.data());
        //std::cout << "AFTER BO" << std::endl;
        //bo data = bo(payload.toAscii().data());
        //be uuid = data.getField("_id");
        //std::cout << "DATAs : " << uuid.toString() << std::endl;


        emit payload(l_payload.toStdString());


        //  Send results to sink
        //message.rebuild();
        //sender.send(message);

        //  Simple progress indicator for the viewer
        //std::cout << "." << std::flush;
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
