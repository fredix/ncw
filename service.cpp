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

#include "service.h"

Service::Service(ncw_params a_ncw) : Worker(), m_ncw(a_ncw)
{
    qDebug() << "Service::Service constructer";
    child_process = new QProcess();

    m_mutex = new QMutex;

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Service::~Service()
{
    delete(child_process);
}


void Service::init()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    m_child_exec = m_ncw.child_exec;
    m_service_name = m_ncw.worker_name;
    m_node_uuid = m_ncw.node_uuid;
    m_node_password = m_ncw.node_password;

    BSONObj tracker = BSON("type" << "service" << "name" << m_service_name.toStdString() << "node_uuid" << m_node_uuid.toStdString() << "command" << m_child_exec.toStdString() << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);

/*    qDebug() << "!!!!   EXEC PROCESS : " << ncw.child_exec;
    child_process->start(m_child_exec);
    bool start = child_process->waitForStarted(30000);

    if (!start)
    {
        qDebug() << "SERVICE IS NOT STARTED";
        qApp->exit();
    }


    connect(child_process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()), Qt::DirectConnection);


    qDebug() << "PID : " << child_process->pid();*/
}



void Service::launch()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    m_child_exec = m_ncw.child_exec;
    m_service_name = m_ncw.worker_name;
    m_node_uuid = m_ncw.node_uuid;
    m_node_password = m_ncw.node_password;


    qDebug() << "!!!!   EXEC PROCESS : " << m_ncw.child_exec;
    /************* START GRUIK CODE *****
    because I must waiting for the pub sub socket is ready **/
    sleep(2);
    /************* END GRUIK CODE   *****/
    child_process->start(m_child_exec);
    bool start = child_process->waitForStarted(30000);

    if (!start)
    {
        qDebug() << "SERVICE IS NOT STARTED";
        qApp->exit();
    }

    if (m_ncw.stdout)
        connect(child_process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()), Qt::DirectConnection);


    qDebug() << "PID : " << child_process->pid();
}


void Service::watchdog()
{
 /*   if (child_process->state() == QProcess::NotRunning)
    {
        ** child is dead, so we exit the worker
        qDebug() << "SERVICE IS NOT RUNNING";
        qApp->exit();
    }
*/
    QDateTime timestamp = QDateTime::currentDateTime();

    BSONObj tracker = BSON("type" << "service" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


//void Service::get_pubsub(bson::bo data)
void Service::get_pubsub(string data)
{
    std::cout << "Service::get_pubsub data : " << data << std::endl;

    QDateTime timestamp = QDateTime::currentDateTime();


    QString payload = QString::fromStdString(data);
    QRegExp filter("([^@]*@).*");

    int pos = filter.indexIn(payload);
    QStringList cap = filter.capturedTexts();

    qDebug() << " cap 1 : " << cap[1];

    payload.remove(cap[1]);

    qDebug() << "PAYLOAD : " << payload;


    BSONObj l_data;
    try {
        //l_data = BSONObj((char*)payload.data());

        l_data = mongo::fromjson(payload.toAscii());

        if (l_data.isValid() && !l_data.isEmpty())
        {
            std::cout << "Service::get_pubsub : " << l_data  << std::endl;


            BSONElement session_uuid = l_data.getField("session_uuid");
            m_session_uuid = QString::fromStdString(session_uuid.str());





            if (l_data.hasField("command"))
            {
                string command = l_data.getField("command").str();
                std::cout << "COMMAND : " << command << std::endl;

                BSONObjBuilder b_datas;

                b_datas << "type" << "service";
                b_datas << "session_uuid" << m_session_uuid.toStdString();
                b_datas << "node_uuid" << m_node_uuid.toStdString();
                b_datas << "node_password" << m_node_password.toStdString();
                b_datas << "name" << m_service_name.toStdString() << "timestamp" << timestamp.toTime_t();

                BSONObj s_datas = b_datas.obj();
                std::cout << "s_datas : " << s_datas << std::endl;



                if (command.compare("get_file") == 0)
                {
                    string filename = l_data.getField("filename").str();

                    std::cout << "Service::s_job_receive filename : " << filename << std::endl;

                    qDebug() << "WORKER SERVICE BEFORE EMIT get_file";
                    bool status = true;
                    emit get_stream(s_datas, filename, &status);
                    qDebug() << "WORKER SERVICE AFTER EMIT get_file";
                    received_file(filename, status);
                }

            }
            else
            {
                //child_process->write(data.toString().data());
                child_process->write(payload.toAscii());
                child_process->write("\n");
                //child_process->waitForBytesWritten(100000);
            }




        }
        else
        {
            std::cout << "DATA NO VALID :" << l_data << std::endl;
        }

    }
    catch (mongo::MsgAssertionException &e)
    {
        std::cout << "error on data : " << l_data << std::endl;
        std::cout << "error on data BSON : " << e.what() << std::endl;
    }


}


void Service::s_job_receive(bson::bo data) {

    std::cout << "Service::s_job_receive DATA : "  << data << std::endl;
    //m_mutex->lock();

    BSONElement session_uuid = data.getFieldDotted("payload.session_uuid");
    m_session_uuid = QString::fromStdString(session_uuid.str());
    BSONElement action = data.getFieldDotted("payload.action");
    //data.getField("step_id").Obj().getObjectID(step_id);

    std::cout << "BE SESSION UUID " << session_uuid << std::endl;
    std::cout << "QS SESSION UUID " << m_session_uuid.toStdString() << std::endl;
    //be step_id;
    //data.getObjectID (step_id);

    std::cout << "RECEIVE MESSAGE : " << data << std::endl;


    QString param;

    QDateTime timestamp = QDateTime::currentDateTime();

    BSONObjBuilder b_tracker;
    b_tracker << "type" << "service";
    b_tracker.append(session_uuid);
    b_tracker << "name" << m_service_name.toStdString() << "action" << "receive" << "timestamp" << timestamp.toTime_t();

    BSONObj tracker = b_tracker.obj();
    emit push_payload(tracker);

    qDebug() << "AFTER EMIT PUSH PAYLOAD";


    BSONObj raw_data = mongo::fromjson(data.getFieldDotted("payload.data").str());
    std::cout << "Service::s_job_receive RAW DATA : " << raw_data << std::endl;




    if (action.str().compare("publish") == 0)
    {

        qDebug() << "PUBLISH !";    

        BSONObjBuilder b_datas;

        b_datas << "action" << "publish";
        b_datas << "gridfs" << false;
        b_datas << "dest" <<  m_service_name.toStdString();
        b_datas << "data" << raw_data;
        b_datas << "payload_type" << raw_data.getFieldDotted("payload_type");
        b_datas << "session_uuid" << m_session_uuid.toStdString();
        b_datas << "name" << m_service_name.toStdString() << "timestamp" << timestamp.toTime_t();

        BSONObj s_datas = b_datas.obj();
        std::cout << "s_datas : " << s_datas << std::endl;

        qDebug() << "WORKER SERVICE BEFORE CREATE PUBSUB PAYLOAD EMIT";
        emit push_payload(s_datas);
        qDebug() << "WORKER SERVICE AFTER CREATE PUBSUB PAYLOAD EMIT";
        return;
    }


    if (data.getField("payload").Obj().hasField("command"))
    {
        string command = data.getFieldDotted("payload.command").str();
        std::cout << "COMMAND : " << command << std::endl;

        BSONObjBuilder b_datas;

        b_datas << "type" << "service";
        b_datas << "session_uuid" << m_session_uuid.toStdString();
        b_datas << "node_uuid" << m_node_uuid.toStdString();
        b_datas << "node_password" << m_node_password.toStdString();
        b_datas << "name" << m_service_name.toStdString() << "timestamp" << timestamp.toTime_t();

        BSONObj s_datas = b_datas.obj();
        std::cout << "s_datas : " << s_datas << std::endl;



        if (command.compare("get_file") == 0)
        {
            string filename = data.getFieldDotted("payload.filename").str();

            std::cout << "Service::s_job_receive filename : " << filename << std::endl;

            qDebug() << "WORKER SERVICE BEFORE EMIT get_file";
            bool status = true;
            emit get_stream(s_datas, filename, &status);
            qDebug() << "WORKER SERVICE AFTER EMIT get_file";
            received_file(filename, status);
        }




    }
    else if (data.getField("payload").Obj().hasField("data"))
    {
        BSONElement r_datas = data.getFieldDotted("payload.data");

        param.append(" ").append(QString::fromStdString(r_datas.str()));

        QByteArray q_datas = r_datas.valuestr();
        qDebug() << "!!!!   SEND PAYLOAD TO STDIN : " << q_datas;

        child_process->write(q_datas);
        child_process->write("\n");
        //child_process->waitForBytesWritten(100000);
    }



}

void Service::received_file(string filename, bool status)
{
    if (status)
    {
        QString json = "{\"received_file\": \"";
        json.append(filename.c_str());
        json.append("\"}");
        child_process->write(json.toAscii());
        child_process->write("\n");

        qDebug() << "RECEIVED FILE " << json;
    }
    else
    {
        QString json = "{\"error\": \"";
        json.append(filename.c_str());
        json.append("\"}");
        child_process->write(json.toAscii());
        child_process->write("\n");

        qDebug() << "ERROR ON RECEIVED FILE " << json;
    }
}



void Service::readyReadStandardOutput()
{
    QByteArray service_stdout = child_process->readAllStandardOutput();
    QString json = service_stdout;
    json = json.simplified();

    std::cout << "STDOUT : " << json.toStdString() << std::endl;

    std::cout << "m_session_uuid : " << m_session_uuid.toStdString() << std::endl;

    BSONObjBuilder b_datas;
    BSONObj b_out;
    QDateTime timestamp = QDateTime::currentDateTime();

    try {
        b_out = mongo::fromjson(json.toAscii());


        std::cout << "b_out : " << b_out << std::endl;

        if (b_out.hasField("action") && (b_out.getField("action").str().compare("push") == 0
                                         || b_out.getField("action").str().compare("publish") == 0
                                         || b_out.getField("action").str().compare("replay") == 0))
        {
            qDebug() << "WORKER SERVICE BEFORE CREATE PAYLOAD EMIT";
            emit push_payload(b_out);
            qDebug() << "WORKER SERVICE AFTER CREATE PAYLOAD EMIT";
            return;
        }
        else if (b_out.hasField("action") && b_out.getField("action").str().compare("get_file") == 0)
        {
            qDebug() << "WORKER SERVICE BEFORE GET FILE PAYLOAD EMIT";

            BSONElement filename =  b_out.getField("filename");

            b_datas << "type" << "service";
            b_datas << "node_uuid" << m_node_uuid.toStdString();
            b_datas << "node_password" << m_node_password.toStdString();
            b_datas << "name" << m_service_name.toStdString() << "timestamp" << timestamp.toTime_t();
            b_datas << "filename" << filename.str();

            BSONObj s_datas = b_datas.obj();
            std::cout << "s_datas : " << s_datas << std::endl;

            qDebug() << "WORKER SERVICE AFTER GET FILE PAYLOAD BEFORE EMIT";

            bool status = true;
            emit get_stream(s_datas, filename.str(), &status);
            qDebug() << "WORKER SERVICE AFTER GET FILE PAYLOAD EMIT";

            received_file(filename.str(), status);
            return;
        }
        else if (!m_session_uuid.isEmpty())
        {
            b_datas << "type" << "service";
            b_datas << "session_uuid" << m_session_uuid.toStdString();
            b_datas << "name" << m_service_name.toStdString() << "action" << "terminate" << "timestamp" << timestamp.toTime_t() << "datas" << b_out;

        }
    }
    catch (mongo::MsgAssertionException &e)
    {
        std::cout << "m_session_uuid : " << m_session_uuid.toStdString() << std::endl;
        std::cout << "m_service_name : " << m_service_name.toStdString() << std::endl;
        std::cout << "error on parsing JSON : " << e.what() << std::endl;
            if (!m_session_uuid.isEmpty())
            {
                b_datas << "type" << "service";
                b_datas << "session_uuid" << m_session_uuid.toStdString();
                b_datas << "name" << m_service_name.toStdString() << "action" << "terminate" << "timestamp" << timestamp.toTime_t() << "datas" << json.toStdString();
            }
    }


    BSONObj s_datas = b_datas.obj();
    std::cout << "s_datas : " << s_datas << std::endl;

    if (s_datas.isValid() && !s_datas.isEmpty())
    {
        qDebug() << "WORKER PROCESS BEFORE EMIT";
        emit push_payload(s_datas);
        qDebug() << "WORKER PROCESS AFTER EMIT";
    }

    //m_mutex->unlock();
}
