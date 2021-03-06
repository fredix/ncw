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

#include "zeromq.h"
#include <fstream>



Zstream::Zstream(zmq::context_t *a_context, QString a_host,  QString a_directory) : m_context(a_context), m_host(a_host), m_directory(a_directory)
{
    std::cout << "Zstream::Zstream construct" << std::endl;

    m_mutex = new QMutex();

    // Prepare our context and socket
    z_message = new zmq::message_t(2);
    z_receive = new zmq::socket_t (*m_context, ZMQ_REQ);

    int hwm = 50000;
    z_receive->setsockopt(ZMQ_SNDHWM, &hwm, sizeof (hwm));
    z_receive->setsockopt(ZMQ_RCVHWM, &hwm, sizeof (hwm));


    std::cout << "Connecting to the ncs stream" << std::endl;

    QString connection_string = "tcp://" + m_host + ":5556";
    z_receive->connect (connection_string.toAscii().data());

/*
    int socket_stream_fd;
    size_t socket_size = sizeof(socket_stream_fd);
    z_receive->getsockopt(ZMQ_FD, &socket_stream_fd, &socket_size);

    qDebug() << "RES getsockopt : " << "res" <<  " FD : " << socket_stream_fd << " errno : " << zmq_strerror (errno);

    check_stream = new QSocketNotifier(socket_stream_fd, QSocketNotifier::Read, this);
    connect(check_stream, SIGNAL(activated(int)), this, SLOT(stream_payload2()), Qt::DirectConnection);
*/
    //data_stream = new QFile("/tmp/nodecast/data_stream");
    //data_stream->open(QIODevice::WriteOnly);
}


Zstream::~Zstream()
{
    std::cout << "Zstream::~Zstream END\r\n" << std::endl;
    z_receive->close();
}


void Zstream::get_stream(BSONObj payload, string filename, bool *status)
{
    //m_mutex->lock ();
   /* if (!Zeromq::lock_get_stream()) return;
    else
    {*/

    std::cout << "Zstream::get_stream, filename : " << filename << " payload : " << payload << std::endl;
/*
    BSONObj z_payload = BSON("payload" << payload << "action" << "getifilename");
    std::cout << "PAYLOAD ADDED FIELD : " << z_payload << std::endl;


    z_message->rebuild(z_payload.objsize());
    memcpy((void* ) z_message->data(), (char*)z_payload.objdata(), z_payload.objsize());
    z_receive->send(*z_message);



    std::cout << "WAIT REPLY !!!!!!!!" << z_payload << std::endl;


    // Get the reply.
    zmq::message_t reply;
    bool res = z_receive->recv (&reply);


    std::cout << "GET REPLY !!!!!!!!" << payload << std::endl;



    BSONObj r_payload = bo((char*)reply.data());

    std::cout << "Received : " << r_payload << std::endl;
    std::cout << "Zstream::get_stream received DATA : " << r_payload << std::endl;



    string filename = r_payload.getField("filename").str();
    */

    QString path = m_directory + "/" + QString::fromStdString(filename);
    //path.append(filename.c_str());


    ofstream out (path.toAscii(),ofstream::binary);


    BSONObj l_payload = BSON("payload" << payload << "action" << "get_file");
    std::cout << "PAYLOAD ADDED FIELD : " << l_payload << std::endl;


    z_message->rebuild(l_payload.objsize());
    memcpy((void* ) z_message->data(), (char*)l_payload.objdata(), l_payload.objsize());
    z_receive->send(*z_message);



//    ofstream out ("/tmp/nodecast/data_stream",ofstream::binary);

    QByteArray data;

    while (true) {

        flush_socket:

        zmq::message_t request;

        bool res = z_receive->recv (&request);

        int64_t more = 0;
        std::size_t more_size = sizeof(more);
        z_receive->getsockopt(ZMQ_RCVMORE, &more, &more_size);

        if (!(more & ZMQ_RCVMORE))
        {
            try {
                BSONObj error_payload = bo((char*)request.data());
                //std::cout << "Zstream::get_stream ERROR : " << error_payload << std::endl;

                if (error_payload.hasField("error"))
                {
                    // Nodecast send an error json : file not found
                    std::cout << "Zstream::get_stream ERROR : " << error_payload << std::endl;
                    out.close ();
                    QFile file(path);
                    file.open(QIODevice::WriteOnly);
                    file.remove();
                    *status = false;
                    break;
                }
            }
            catch (mongo::MsgAssertionException &e)
            {
                std::cout << "RECEIVE LAST CHUNK" << std::endl;
            }
        }



        //QByteArray data = QByteArray::fromBase64((char*) request.data());
        data.append((char*) request.data(), request.size());

        //std::cout << "Zstream::stream_payload received request: [" << data << "]" << std::endl;

        std::cout << "Zstream::stream_payload received DATA size : " << request.size() << std::endl;



//          char *plop = (char*) request.data();
        if (data.size() == 0) {
            std::cout << "Zstream::stream_payload received request 0" << std::endl;
            break;
        }


        //QTextStream out(data_stream);
        //out << data;
        out.write( data.constData() , request.size());

        data.clear();


        if (!(more & ZMQ_RCVMORE)) break;


    }
    out.close ();   
  //  m_mutex->unlock ();

   // Zeromq::unlock_get_stream();
//}
    //delete(z_message);

    /* Get the reply.
    zmq::message_t reply;
    z_stream->recv (&reply);
    std::cout << "Received stream response : " << (char*) reply.data() << std::endl;*/
}



void Zstream::stream_payload2()
{
    check_stream->setEnabled(false);

    std::cout << "Zstream::stream_payload2" << std::endl;

    qint32 events = 0;
    std::size_t eventsSize = sizeof(events);
    z_receive->getsockopt(ZMQ_EVENTS, &events, &eventsSize);



    if (events & ZMQ_POLLIN)
    {
        std::cout << "Zstream::stream_payload2 ZMQ_POLLIN" <<  std::endl;

        //QFile dstream("/tmp/nodecast/data_stream");
        //dstream.open(QIODevice::WriteOnly);

        //QDataStream out(&dstream);

        ofstream out ("/tmp/nodecast/data_stream",ofstream::binary);



        while (true) {

            flush_socket:

            zmq::message_t request;

            bool res = z_receive->recv (&request, ZMQ_NOBLOCK);
            if (!res && zmq_errno () == EAGAIN) break;

            if (request.size() == 0) {
                std::cout << "Zstream::stream_payload2 received request 0" << std::endl;
                break;
            }

            BSONObj data = bo((char*) request.data());

            //std::cout << "Zstream::stream_payload received request: [" << data << "]" << std::endl;

            std::cout << "Zstream::stream_payload2 received DATA : " << data << std::endl;



//            out.write( data.toString().c_str() , data.objsize());



        }
  //      out.close ();
    }
    std::cout << "Zstream::stream_payload2 END" << std::endl;

    check_stream->setEnabled(true);
    //m_mutex->unlock ();
}


void Zstream::stream_payload()
{
    check_stream->setEnabled(false);

    std::cout << "Zstream::stream_payload" << std::endl;

    qint32 events = 0;
    std::size_t eventsSize = sizeof(events);
    z_receive->getsockopt(ZMQ_EVENTS, &events, &eventsSize);



    if (events & ZMQ_POLLIN)
    {
        std::cout << "Zstream::stream_payload ZMQ_POLLIN" <<  std::endl;

        //QFile dstream("/tmp/nodecast/data_stream");
        //dstream.open(QIODevice::WriteOnly);

        //QDataStream out(&dstream);

        ofstream out ("/tmp/nodecast/data_stream",ofstream::binary);



        while (true) {

            flush_socket:

            zmq::message_t request;

            bool res = z_receive->recv (&request, ZMQ_NOBLOCK);

            int64_t more = 0;
            std::size_t more_size = sizeof(more);
            z_receive->getsockopt(ZMQ_RCVMORE, &more, &more_size);


            if (request.size() == 0) {
                std::cout << "Zstream::stream_payload received request 0" << std::endl;
                break;
            }

            const char * data = (char*) request.data();


            //QTextStream out(data_stream);            
            //out << data;
            out.write( data , strlen(data));



            if (!(more & ZMQ_RCVMORE)) break;

/*
            bo ping = BSON("payload" << BSON("type" << "init" << "action" << "ping"));

            reply.rebuild(ping.objsize());
            memcpy ((void *) reply.data (), (char*)ping.objdata(), ping.objsize());
          //  std::cout << "Sending Hello " << request_nbr << "…" << std::endl;
            z_receive->send (reply);
*/


            //std::cout << "RECEIVE DATA : " << request.data() << std::endl;


            /*
            BSONObj l_payload;
            try {
                l_payload = bo((char*)request.data());
                std::cout << "Zstream Received payload : " << l_payload << std::endl;
            }
            catch (mongo::MsgAssertionException &e)
            {
                std::cout << "error on data : " << l_payload << std::endl;
                std::cout << "error on data BSON : " << e.what() << std::endl;
                break;
            }
            */

            /*
            QString payload_action = QString::fromStdString(l_payload.getFieldDotted("payload.action").str());

            if (payload_action == "get_file")
            {

                BSONObj session_uuid = BSON("uuid" << l_payload.getField("session_uuid").str());

                BSONObj session = nosql_->Find("sessions", session_uuid);

                std::cout << "!!!!! session !!!!!! : " << session << std::endl;

                BSONObj payload_id = BSON("_id" << session.getField("payload_id").OID());
                std::cout << "payload_id : " << payload_id << std::endl;

                BSONObj payload = nosql_->Find("payloads", payload_id);






                BSONObjBuilder b_payload;
                b_payload.append(l_payload.getField("session_uuid"));
                b_payload << "file_path" << "/tmp/nodecast/dump_gridfile";
                BSONObj r_payload = b_payload.obj();


                z_message->rebuild(r_payload.objsize());
                memcpy(z_message->data(), (char*)r_payload.objdata(), r_payload.objsize());
                z_stream->send(*z_message);
            }*/

        }
        //out << "\n";
        //dstream.close ();
        out.close ();

        /*zmq::message_t reply;
        bo ping = BSON("type" << "ack" << "action" << "receive");
        reply.rebuild(ping.objsize());
        memcpy ((void *) reply.data (), (char*)ping.objdata(), ping.objsize());
        z_receive->send (reply);*/

        qDebug() << "BREAK ZEROMQ RECV";;
    }

    check_stream->setEnabled(true);
    //m_mutex->unlock ();
}

Ztracker::Ztracker(zmq::context_t *a_context, QString a_host, QString a_port) : m_context(a_context)
{
    std::cout << "Ztracker::Ztracker construct" << std::endl;

    m_mutex = new QMutex();

    m_host = a_host;
    m_port = a_port;

    // Prepare our context and socket        
    z_message = new zmq::message_t(2);

    z_sender = new zmq::socket_t (*m_context, ZMQ_REQ);

    int hwm = 50000;
    z_sender->setsockopt(ZMQ_SNDHWM, &hwm, sizeof (hwm));
    z_sender->setsockopt(ZMQ_RCVHWM, &hwm, sizeof (hwm));


    //int linger = 0;
    //z_sender->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));
}


void Ztracker::init()
{
    m_mutex->lock();

    std::cout << "Connecting to the ncs tracker : " << m_host.toStdString() << std::endl;
    //z_sender->connect ("tcp://localhost:5569");

    QString connection_string = "tcp://" + m_host + ":" + m_port;
    z_sender->connect (connection_string.toAscii().data());

    // Do 10 requests, waiting each time for a response
    //for (int request_nbr = 0; request_nbr != 2; request_nbr++) {

    bo ping = BSON("payload" << BSON("type" << "init" << "action" << "ping"));

    z_message->rebuild(ping.objsize());
    memcpy ((void *) z_message->data (), (char*)ping.objdata(), ping.objsize());
  //  std::cout << "Sending Hello " << request_nbr << "…" << std::endl;
    z_sender->send (*z_message);
    //delete(z_message);

    // Get the reply.
    zmq::message_t reply;
    z_sender->recv (&reply);
    std::cout << "Received : " << (char*) reply.data() << std::endl;


    m_mutex->unlock();

      /*  if (s_interrupted) {
                    printf ("W: interrupt received, killing server…\n");
                    qApp->exit();
                }*/

    //}
}

Ztracker::~Ztracker()
{
    std::cout << "Ztracker::~Ztracker END\r\n" << std::endl;
    z_sender->close();
}



void Ztracker::push_tracker(bson::bo payload)
{
    //m_mutex->lock();
    std::cout << "Ztracker::push_tracker" << std::endl;
    std::cout << "TRACKER : " << payload << std::endl;

    qDebug() << "SENDER : " << this->sender();



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
    std::cout << "PUSH PAYLOAD : " <<  l_payload << std::endl;

    z_message->rebuild(l_payload.objsize());
    memcpy(z_message->data(), (char*)l_payload.objdata(), l_payload.objsize());
    z_sender->send(*z_message);
    //delete(z_message);
    /************************/

    // Get the reply.
    zmq::message_t reply;
    z_sender->recv (&reply);

    BSONObj r_payload = BSONObj((const char*)reply.data());

    std::cout << "Received : " << r_payload << std::endl;

    if (r_payload.hasField("uuid"))
    {
        std::cout << "UUID : " << r_payload["uuid"] << std::endl;
        m_uuid = QString::fromStdString(r_payload["uuid"].str());
        m_worker_port = QString::number(r_payload["port"].numberInt());

        emit worker_port(m_worker_port, m_uuid);
    }
    else
    {
        std::cout << "STATUS : " << r_payload["status"] << std::endl;
    }


    //m_mutex->unlock();
}


Zpayload::~Zpayload()
{
    std::cout << "Zpayload::Zpayload destruct" << std::endl;
    m_receiver->close();
    std::cout << "Zpayload::Zpayload m_receiver" << std::endl;

    m_socket_worker->close();
    std::cout << "Zpayload::Zpayload m_socket_worker" << std::endl;

    m_socket_pubsub->close();
    std::cout << "Zpayload::Zpayload m_socket_pubsub" << std::endl;

}



Zpayload::Zpayload(zmq::context_t *a_context, ncw_params ncw, QString a_ncs_ip) : m_context(a_context), m_host(a_ncs_ip), m_worker_name(ncw.worker_name), m_node_uuid(ncw.node_uuid), m_node_password(ncw.node_password), m_ncw(ncw)
{
    std::cout << "Zpayload::Zpayload construct" << std::endl;

    QString connection_string = "tcp://" + m_host + ":5555";
    QByteArray t_connection_string = connection_string.toAscii();

    m_socket_worker = new zmq::socket_t (*m_context, ZMQ_PUSH);
    int hwm = 50000;
    m_socket_worker->setsockopt(ZMQ_SNDHWM, &hwm, sizeof (hwm));
    m_socket_worker->setsockopt(ZMQ_RCVHWM, &hwm, sizeof (hwm));

    //int linger = 0;
    //m_socket_worker->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));


    m_socket_worker->connect(t_connection_string.constData());



}

void Zpayload::pubsub_payload()
{
    check_pubsub_payload->setEnabled(false);

    qDebug() << "Zpayload::pubsub_payload from : " << m_host;
    /*
    qint32 events = 0;
    std::size_t eventsSize = sizeof(events);
    m_socket_pubsub->getsockopt(ZMQ_EVENTS, &events, &eventsSize);
    */
    zmq::poll (&m_items[0], 2, 0);

    //if (events & ZMQ_POLLIN)
    if (m_items[1].revents & ZMQ_POLLIN)
    {
        std::cout << "Zpayload::pubsub_payload ZMQ_POLLIN" <<  std::endl;

        while (true) {
            zmq::message_t request;

            bool res = m_socket_pubsub->recv (&request, ZMQ_NOBLOCK);
            if (res == -1 && zmq_errno () == EAGAIN) break;

            if (request.size() == 0) {
                std::cout << "Zpayload::pubsub_payload received request 0" << std::endl;
                break;
            }

            //char *payload = (char*) request.data();

            QString raw_data = QString::fromAscii((char*) request.data(), request.size());
            qDebug() << "RAW DATA : " << raw_data;

            emit emit_pubsub(raw_data);

            /*
            BSONObj data;
            try {
                //data = BSONObj((char*) request.data());
                //data = mongo::fromjson(raw_data.toAscii());

                if (data.isValid() && !data.isEmpty())
                {
                    std::cout << "Zpayload::pubsub_payload received : " << res << " data : " << data  << std::endl;

                    std::cout << "!!!!!!! BEFORE FORWARD PAYLOAD !!!!" << std::endl;
                    emit emit_pubsub(data.copy());
                    std::cout << "!!!!!!! AFTER FORWARD PAYLOAD !!!!" << std::endl;
                }
                else
                {
                    std::cout << "DATA NO VALID !" << std::endl;
                    break;
                }

            }
            catch (mongo::MsgAssertionException &e)
            {
                std::cout << "error on data : " << data << std::endl;
                std::cout << "error on data BSON : " << e.what() << std::endl;
                break;
            }
            */
        }
    }
    check_pubsub_payload->setEnabled(true);
}


void Zpayload::init_payload(QString worker_port, QString worker_uuid)
{
    //QString connection_string = "tcp://" + m_host + ":" + m_port;
    QString connection_string = "tcp://" + m_host + ":" + worker_port;

    m_uuid = worker_uuid;

    qDebug() << "connection_string : " << connection_string;
    m_message = new zmq::message_t(2);

    m_receiver = new zmq::socket_t(*m_context, ZMQ_PULL);
    int hwm = 50000;
    m_receiver->setsockopt(ZMQ_SNDHWM, &hwm, sizeof (hwm));
    m_receiver->setsockopt(ZMQ_RCVHWM, &hwm, sizeof (hwm));

    //m_receiver->setsockopt(ZMQ_IDENTITY, worker_uuid.toStdString(),  worker_uuid.toStdString().size());


    QByteArray t_connection_string = connection_string.toAscii();

    m_receiver->connect(t_connection_string.constData());
    std::cout << "receiver connect error : " << zmq_strerror (errno) << std::endl;

    int payload_socket_fd;
    size_t socket_size = sizeof(payload_socket_fd);
    m_receiver->getsockopt(ZMQ_FD, &payload_socket_fd, &socket_size);

    qDebug() << "RES getsockopt : " << "res" <<  " FD : " << payload_socket_fd << " errno : " << zmq_strerror (errno);

    m_items[0].socket = (void *)*m_receiver;
    //m_items[0].socket = NULL;
    //m_items[0].fd = payload_socket_fd;
    m_items[0].events = ZMQ_POLLIN;


    check_receive_payload = new QSocketNotifier(payload_socket_fd, QSocketNotifier::Read, this);
    connect(check_receive_payload, SIGNAL(activated(int)), this, SLOT(receive_payload()));


    /************ PUBSUB SOCKET ***************/
    QString connection_pubsub_string = "tcp://" + m_host + ":5557";
    QByteArray t_connection_pubsub_string = connection_pubsub_string.toAscii();

    QByteArray filter1 = m_worker_name.toAscii() + " ";
    QByteArray filter2 = m_node_uuid.toAscii() + " ";
    QByteArray filter3 = m_node_uuid.toAscii() + "." + m_worker_name.toAscii() + " ";
    QByteArray filter4 = m_node_uuid.toAscii() + "." + m_worker_name.toAscii() + "." + m_uuid.toAscii() + " ";

    qDebug() << "FILTER1 : " << filter1;
    qDebug() << "FILTER2 : " << filter2;
    qDebug() << "FILTER3 : " << filter3;
    qDebug() << "FILTER4 : " << filter4;

    m_socket_pubsub = new zmq::socket_t (*m_context, ZMQ_SUB);
    int pub_hwm = 50000;
    m_socket_pubsub->setsockopt(ZMQ_SNDHWM, &pub_hwm, sizeof (pub_hwm));
    m_socket_pubsub->setsockopt(ZMQ_RCVHWM, &pub_hwm, sizeof (pub_hwm));

    //int linger = 0;
    //m_socket_pubsub->setsockopt (ZMQ_LINGER, &linger, sizeof (linger));

    m_socket_pubsub->setsockopt(ZMQ_SUBSCRIBE, filter1.data(), filter1.size());
    m_socket_pubsub->setsockopt(ZMQ_SUBSCRIBE, filter2.data(), filter2.size());
    m_socket_pubsub->setsockopt(ZMQ_SUBSCRIBE, filter3.data(), filter3.size());
    m_socket_pubsub->setsockopt(ZMQ_SUBSCRIBE, filter4.data(), filter4.size());

    m_socket_pubsub->connect(t_connection_pubsub_string.constData());


    int pubsub_payload_socket_fd;
    size_t pubsub_socket_size = sizeof(pubsub_payload_socket_fd);
    m_socket_pubsub->getsockopt(ZMQ_FD, &pubsub_payload_socket_fd, &pubsub_socket_size);

    qDebug() << "RES getsockopt : " << "res" <<  " FD : " << pubsub_payload_socket_fd << " errno : " << zmq_strerror (errno);


    m_items[1].socket = (void *)*m_socket_pubsub;
    //m_items[1].socket = NULL;
    //m_items[1].fd = pubsub_payload_socket_fd;
    m_items[1].events = ZMQ_POLLIN;


    check_pubsub_payload = new QSocketNotifier(pubsub_payload_socket_fd, QSocketNotifier::Read, this);
    connect(check_pubsub_payload, SIGNAL(activated(int)), this, SLOT(pubsub_payload()), Qt::DirectConnection);
    /******************************************/




    // when the pubsub socket is connected (but in fact not ready) I launch the worker
    emit emit_launch_worker(m_ncw);
}


void Zpayload::push_payload(BSONObj data)
{
  /*  if (!Zeromq::lock_push_payload()) return;
    else
    {*/

        /****** PUSH API PAYLOAD *******/
        std::cout << "Zpayload:: PUSH PAYLOAD : " <<  data << std::endl;

        BSONObj l_payload = BSON("payload" << data << "worker_name" << m_worker_name.toStdString() << "uuid" << m_uuid.toStdString() << "node_uuid" << m_node_uuid.toStdString() << "node_password" << m_node_password.toStdString());

        std::cout << "PAYLOAD ADDED FIELD : " << l_payload << std::endl;


        m_message->rebuild(l_payload.objsize());
        memcpy(m_message->data(), (char*)l_payload.objdata(), l_payload.objsize());
        m_socket_worker->send(*m_message);
        //delete(m_message);
        /************************/

      //  Zeromq::unlock_push_payload();
   // }
}

void Zpayload::receive_payload()
{
    check_receive_payload->setEnabled(false);

    std::cout << "Zpayload::receive_payload" << std::endl;

    /*
    qint32 events = 0;
    std::size_t eventsSize = sizeof(events);
    m_receiver->getsockopt(ZMQ_EVENTS, &events, &eventsSize);
    std::cout << "Zpayload::receive_payload ZMQ_EVENTS : " <<  events << std::endl;
    */

    zmq::poll (&m_items[0], 2, 0);


    //if (events & ZMQ_POLLIN)
    if (m_items[0].revents & ZMQ_POLLIN)
    {
        std::cout << "Zpayload::receive_payload ZMQ_POLLIN" <<  std::endl;

        while (true)
        {
            flush_socket:

            zmq::message_t request;
            bool res = m_receiver->recv(&request, ZMQ_NOBLOCK);
            if (!res && zmq_errno () == EAGAIN) break;

            if (request.size() == 0) {
                std::cout << "Zpayload::worker_response received request 0" << std::endl;
                break;
            }


            BSONObj data;
            try {
                data = BSONObj((char*)request.data());

                if (data.isValid() && !data.isEmpty())
                {
                    std::cout << "Zpayload received : " << res << " data : " << data  << std::endl;

                    std::cout << "!!!!!!! BEFORE FORWARD PAYLOAD !!!!" << std::endl;
                    emit emit_payload(data.copy());
                    std::cout << "!!!!!!! AFTER FORWARD PAYLOAD !!!!" << std::endl;
                }
                else
                {
                    std::cout << "DATA NO VALID !" << std::endl;
                }

            }
            catch (mongo::MsgAssertionException &e)
            {
                std::cout << "error on data : " << data << std::endl;
                std::cout << "error on data BSON : " << e.what() << std::endl;
                goto flush_socket;
            }
        }

    }

    check_receive_payload->setEnabled(true);
}


Zdispatcher *Zdispatcher::_singleton = NULL;


Zdispatcher* Zdispatcher::getInstance() {
      return _singleton;
}

typedef QSharedPointer<Zeromq> Zeromq_pushPtr;

Zdispatcher::Zdispatcher(ncw_params ncw, QString ncs_ips)
{
    qDebug() << "Zdispatcher::Zdispatcher constructor";

    qRegisterMetaType<bson::bo>("bson::bo");
    qRegisterMetaType<string>("string");
    qRegisterMetaType<ncw_params>("ncw_params");

    ncs_counter = 0;
    ncs_number = 0;

    z_context = new zmq::context_t(1);

  //  thread_service = new QThread;
    ncw_service = new Service(z_context, ncw);
    connect(ncw_service, SIGNAL(push_payload(bson::bo)), this, SLOT(push_payload(bson::bo)));
    connect(ncw_service, SIGNAL(get_stream(bson::bo, string, bool*)), this, SLOT(push_stream(bson::bo, string, bool*)));
  //  connect(thread_service, SIGNAL(started()), ncw_service, SLOT(init()));
  //  connect(ncw_service, SIGNAL(destroyed()), thread_service, SLOT(quit()), Qt::DirectConnection);

//    ncw_service->moveToThread(thread_service);
//    thread_service->start();




    QStringList ips = ncs_ips.split(",");
    ncs_number = ips.size();
    foreach (QString ip, ips)
    {
        zeromq_push[ncs_counter] = QSharedPointer<Zeromq> (new Zeromq(z_context, ncw, ip));


        connect(ncw_service, SIGNAL(return_tracker(bson::bo)), zeromq_push[ncs_counter].data()->tracker, SLOT(push_tracker(bson::bo)), Qt::QueuedConnection);

        connect(zeromq_push[ncs_counter].data()->payload, SIGNAL(emit_launch_worker(ncw_params)), ncw_service, SLOT(launch()), Qt::QueuedConnection);
        connect(zeromq_push[ncs_counter].data()->payload, SIGNAL(emit_payload(bson::bo)), ncw_service, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);
        connect(zeromq_push[ncs_counter].data()->payload, SIGNAL(emit_pubsub(QString)), ncw_service, SLOT(get_pubsub(QString)), Qt::QueuedConnection);

        ncs_counter += 1;
    }


    Service::init(ncw);
}

Zdispatcher::~Zdispatcher()
{
    qDebug() << "Zdispatcher::~Zdispatcher";
    ncw_service->deleteLater();
    //thread_service->wait();

    zeromq_push.clear();
    z_context->close();
    qDebug() << "Zdispatcher::~Zdispatcher z_context close";
}


void Zdispatcher::push_payload(bson::bo data)
{
    int round_robin = qrand() % ncs_number;
    qDebug() << "Zdispatcher::push_payload ROUND ROBIN : " << round_robin;
    zeromq_push[round_robin].data()->payload->push_payload(data);
}

void Zdispatcher::push_stream(bson::bo payload, string filename, bool *status)
{
    int round_robin = qrand() % ncs_number;
    qDebug() << "Zdispatcher::push_stream ROUND ROBIN : " << round_robin;
    zeromq_push[round_robin].data()->zstream->get_stream(payload, filename, status);

}



Zeromq::~Zeromq()
{
    tracker->deleteLater();
    thread_tracker->wait();
    qDebug() << "Zeromq::~Zeromq thread_tracker deleted";

    payload->deleteLater();
    thread_payload->wait();
    qDebug() << "Zeromq::~Zeromq thread_payload deleted";

    zstream->deleteLater();
    thread_stream->wait();
    qDebug() << "Zeromq::~Zeromq thread_stream deleted";

    //ztracker_push.clear();
    //q_thread_tracker.clear();
    /*
    foreach(QThread *child, q_thread_tracker)
    {
        child->wait();
    }*/

    //zpayload_push.clear();
    //q_thread_payload.clear();
    /*foreach(QThread *child, q_thread_payload)
    {
        child->wait();
    }*/

    //zstream_push.clear();
    //q_thread_stream.clear();
    /*foreach(QThread *child, q_thread_stream)
    {
        child->wait();
    }*/

    //delete(ncw_service);
    //ncw_service->deleteLater();
    //delete(m_context);
    qDebug() << "Zeromq DELETE !";
}

//QQueue z_queue;
bool Zeromq::check_push_payload = false;
bool Zeromq::check_get_stream = false;

QMutex Zeromq::mutex_push_payload;
QMutex Zeromq::mutex_get_stream;

bool Zeromq::lock_push_payload()
{
    QMutexLocker locker(&mutex_push_payload);

    if (check_push_payload == true)
        return false;
    else
    {
        check_push_payload = true;
        return true;
    }
}


void Zeromq::unlock_push_payload()
{
    QMutexLocker locker(&mutex_push_payload);
    check_push_payload = false;
}

bool Zeromq::lock_get_stream()
{
    QMutexLocker locker(&mutex_get_stream);

    if (check_get_stream == true)
        return false;
    else
    {
        check_get_stream = true;
        return true;
    }
}


void Zeromq::unlock_get_stream()
{
    QMutexLocker locker(&mutex_get_stream);
    check_get_stream = false;
}




Zeromq::Zeromq(zmq::context_t *a_context, ncw_params a_ncw, QString a_ncs_ip) : m_context(a_context), m_ncw(a_ncw), m_ncs_ip(a_ncs_ip)
{
    qDebug() << "Zeromq::construct";
    StringToEnumMap enumToWorker;

    enumToWorker.insert(QString("service"), WSERVICE);
    enumToWorker.insert(QString("process"), WPROCESS);


/*

    if (!(ncw_dispatcher = Zdispatcher::getInstance ()))
        ncw_dispatcher = new Zdispatcher();
    ncw_dispatcher->ncs_counter += 1;
*/


  /*  m_context = new zmq::context_t(1);

    QStringList ips = m_ncs_ip.split(",");
    foreach (QString ncs_ip, ips)
    {
*/
        /*********** TRACKER ***********/
        /*
        q_thread_tracker[ncs_ip] = QSharedPointer<QThread> (new QThread(this));
        ztracker_push[ncs_ip] =  QSharedPointer<Ztracker> (new Ztracker(m_context, ncs_ip, m_ncw.ncs_port));

        connect(q_thread_tracker[ncs_ip].data(), SIGNAL(started()), ztracker_push[ncs_ip].data(), SLOT(init()));
        connect(ztracker_push[ncs_ip].data(), SIGNAL(destroyed()), q_thread_tracker[ncs_ip].data(), SLOT(quit()), Qt::DirectConnection);

        ztracker_push[ncs_ip]->moveToThread(q_thread_tracker[ncs_ip].data());
        q_thread_tracker[ncs_ip].data()->start();
        */

        thread_tracker = new QThread(this);
        tracker = new Ztracker(m_context, m_ncs_ip, m_ncw.ncs_port);
        connect(thread_tracker, SIGNAL(started()), tracker, SLOT(init()));
        connect(tracker, SIGNAL(destroyed()), thread_tracker, SLOT(quit()), Qt::DirectConnection);

        tracker->moveToThread(thread_tracker);
        thread_tracker->start();



        /*********** TRACKER ***********/


        /*********** PAYLOAD ***********/
        /*
        q_thread_payload[ncs_ip] = QSharedPointer<QThread> (new QThread(this));
        zpayload_push[ncs_ip] =  QSharedPointer<Zpayload> (new Zpayload(m_context, m_ncw, m_ncw.ncs_port));

        connect(q_thread_payload[ncs_ip].data(), SIGNAL(started()), zpayload_push[ncs_ip].data(), SLOT(init()));
        connect(zpayload_push[ncs_ip].data(), SIGNAL(destroyed()), q_thread_payload[ncs_ip].data(), SLOT(quit()), Qt::DirectConnection);

        zpayload_push[ncs_ip]->moveToThread(q_thread_payload[ncs_ip].data());
        q_thread_payload[ncs_ip].data()->start();
        */


        thread_payload = new QThread(this);
        payload = new Zpayload(m_context, m_ncw, m_ncs_ip);
//        ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter] = QSharedPointer<Zpayload> (new Zpayload(m_context, m_ncw, m_ncw.ncs_port));
        //zpayload_push[ncs_ip] =  QSharedPointer<Zpayload> (new Zpayload(m_context, m_ncw, m_ncw.ncs_port));


 //       connect(thread_payload, SIGNAL(started()), payload, SLOT(receive_payload()));
        connect(payload, SIGNAL(destroyed()), thread_payload, SLOT(quit()), Qt::DirectConnection);
        connect(tracker, SIGNAL(worker_port(QString, QString)), payload, SLOT(init_payload(QString, QString)));


//        connect(ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter].data(), SIGNAL(destroyed()), thread_payload, SLOT(quit()), Qt::DirectConnection);
//        connect(tracker, SIGNAL(worker_port(QString, QString)), ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter].data(), SLOT(init_payload(QString, QString)));



   //     connect(thread_payload, SIGNAL(started()), payload, SLOT(receive_payload()));
        //ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter].data()->moveToThread(thread_payload);



        payload->moveToThread(thread_payload);
        thread_payload->start();


        /*********** PAYLOAD ***********/



        /*********** STREAM ***********/
        /*
        q_thread_stream[ncs_ip] = QSharedPointer<QThread> (new QThread(this));
        zstream_push[ncs_ip] =  QSharedPointer<Zstream> (new Zstream(m_context, ncs_ip, m_ncw.directory));

        connect(q_thread_stream[ncs_ip].data(), SIGNAL(started()), zstream_push[ncs_ip].data(), SLOT(init()));
        connect(zstream_push[ncs_ip].data(), SIGNAL(destroyed()), q_thread_stream[ncs_ip].data(), SLOT(quit()), Qt::DirectConnection);

        zstream_push[ncs_ip]->moveToThread(q_thread_stream[ncs_ip].data());
        q_thread_stream[ncs_ip].data()->start();
        */


        thread_stream = new QThread;
        //payload = new Zpayload(m_context, m_host, m_port);

        zstream = new Zstream(m_context, m_ncs_ip, m_ncw.directory);
        //ncw_dispatcher->zstream[ncw_dispatcher->ncs_counter] = QSharedPointer<Zstream> (new Zstream(m_context, m_ncs_ip, m_ncw.directory));

        connect(zstream, SIGNAL(destroyed()), thread_stream, SLOT(quit()), Qt::DirectConnection);

        //connect(thread_payload, SIGNAL(started()), payload, SLOT(receive_payload()));
        zstream->moveToThread(thread_stream);
        thread_stream->start();
        //connect(tracker, SIGNAL(worker_port(QString, QString)), stream, SLOT(init_payload(QString, QString)));

        /*********** STREAM ***********/


        switch (enumToWorker[a_ncw.worker_type])
        {
        case WSERVICE:
        {
            qDebug() << "WSERVICE : " << a_ncw.worker_type ;

    //        QThread *thread_service = new QThread;




  //          if (!(ncw_service = Service::getInstance ()))
  //              ncw_service = new Service(m_context, a_ncw);

    //        connect(payload, SIGNAL(emit_payload(bson::bo)), ncw_service, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);
    //        connect(payload, SIGNAL(emit_pubsub(QString)), ncw_service, SLOT(get_pubsub(QString)), Qt::QueuedConnection);


//            connect(ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter].data(), SIGNAL(emit_payload(bson::bo)), ncw_service, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);
//            connect(ncw_dispatcher->zpayload[ncw_dispatcher->ncs_counter].data(), SIGNAL(emit_pubsub(QString)), ncw_service, SLOT(get_pubsub(QString)), Qt::QueuedConnection);


        //    connect(ncw_service, SIGNAL(return_tracker(bson::bo)), tracker, SLOT(push_tracker(bson::bo)), Qt::QueuedConnection);



       //     connect(ncw_service, SIGNAL(push_payload(bson::bo)), ncw_dispatcher, SLOT(push_payload(bson::bo)), Qt::QueuedConnection);
       //     connect(ncw_service, SIGNAL(get_stream(bson::bo, string, bool*)), ncw_dispatcher, SLOT(push_stream(bson::bo, string, bool*)), Qt::QueuedConnection);


         //   connect(ncw_service, SIGNAL(push_payload(bson::bo)), payload, SLOT(push_payload(bson::bo)), Qt::QueuedConnection);
         //   connect(ncw_service, SIGNAL(get_stream(bson::bo, string, bool*)), zstream, SLOT(get_stream(bson::bo, string, bool*)), Qt::BlockingQueuedConnection);

     //       connect(payload, SIGNAL(emit_launch_worker(ncw_params)), ncw_service, SLOT(launch()), Qt::QueuedConnection);

      //      connect(thread_service, SIGNAL(started()), ncw_service, SLOT(init()));
      //      ncw_service->moveToThread(thread_service);
      //      thread_service->start();

        //   ncw_service->init();
            break;
        }

        case WPROCESS:
            qDebug() << "WPROCESS : " << a_ncw.worker_type ;

            ncw_process = new Process(a_ncw);

            connect(payload, SIGNAL(emit_payload(bson::bo)), ncw_process, SLOT(s_job_receive(bson::bo)), Qt::QueuedConnection);

            connect(ncw_process, SIGNAL(return_tracker(bson::bo)), tracker, SLOT(push_tracker(bson::bo)), Qt::QueuedConnection);
            connect(ncw_process, SIGNAL(push_payload(bson::bo)), payload, SLOT(push_payload(bson::bo)), Qt::QueuedConnection);


            //connect(payload, SIGNAL(emit_launch_worker(ncw_params)), ncw_process, SLOT(init(ncw_params)), Qt::QueuedConnection);

            ncw_process->init();
            break;

        default:
            qDebug() << "worker unknown : " << a_ncw.worker_type ;
            qApp->exit (1);
        }


   // }


}

