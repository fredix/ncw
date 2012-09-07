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

Service::Service() : Worker()
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


//void Service::init(QString child_exec, QString a_service_name)
void Service::init(ncw_params ncw)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    m_child_exec = ncw.child_exec;
    m_service_name = ncw.worker_name;
    m_node_uuid = ncw.node_uuid;
    m_node_password = ncw.node_password;

    BSONObj tracker = BSON("type" << "service" << "name" << m_service_name.toStdString() << "command" << m_child_exec.toStdString() << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);

    qDebug() << "!!!!   EXEC PROCESS : " << ncw.child_exec;
    child_process->start(m_child_exec);

    connect(child_process,SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()), Qt::DirectConnection);


    qDebug() << "PID : " << child_process->pid();
}


void Service::watchdog()
{
    if (child_process->state() == QProcess::NotRunning)
    {
        /*** child is dead, so we exit the worker */
        qDebug() << "CHILD IS DEAD";
        qApp->exit();
    }

    QDateTime timestamp = QDateTime::currentDateTime();

    BSONObj tracker = BSON("type" << "service" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


//void Service::get_pubsub(bson::bo data)
void Service::get_pubsub(string data)
{
    std::cout << "Service::get_pubsub data : " << data << std::endl;

    QString payload = QString::fromStdString(data);

    QRegExp filter("([^@]*@).*");
    payload.remove(filter.cap(1));

    qDebug() << "PAYLOAD : " << payload;

    //child_process->write(data.toString().data());
    child_process->write(payload.toAscii());
    child_process->write("\n");
    //child_process->waitForBytesWritten(100000);
}


void Service::s_job_receive(bson::bo data) {

    qDebug() << "Service::s_job_receive";
    //m_mutex->lock();

    BSONElement r_datas = data.getField("data");
    BSONElement session_uuid = data.getField("session_uuid");
    m_session_uuid = QString::fromStdString(session_uuid.str());
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

    param.append(" ").append(QString::fromStdString(r_datas.str()));


    QByteArray q_datas = r_datas.valuestr();
    qDebug() << "!!!!   SEND PAYLOAD TO STDIN : " << q_datas;

    child_process->write(q_datas);
    child_process->write("\n");
    //child_process->waitForBytesWritten(100000);
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

        if (b_out.hasField("action") && (b_out.getField("action").str().compare("create") == 0                                      
                                         || b_out.getField("action").str().compare("publish") == 0
                                         || b_out.getField("action").str().compare("replay") == 0))
        {
            qDebug() << "WORKER SERVICE BEFORE CREATE PAYLOAD EMIT";
            emit push_payload(b_out);
            qDebug() << "WORKER SERVICE AFTER CREATE PAYLOAD EMIT";
            return;
        }
        else if (!m_session_uuid.isEmpty() && b_out.hasField("action") && b_out.getField("action").str().compare("get_file") == 0)
        {
            qDebug() << "WORKER SERVICE BEFORE GET FILE PAYLOAD EMIT";

            b_datas << "type" << "service";
            b_datas << "session_uuid" << m_session_uuid.toStdString();
            b_datas << "node_uuid" << m_node_uuid.toStdString();
            b_datas << "node_password" << m_node_password.toStdString();
            b_datas << "name" << m_service_name.toStdString() << "action" << "get_file" << "timestamp" << timestamp.toTime_t();

            BSONObj s_datas = b_datas.obj();
            std::cout << "s_datas : " << s_datas << std::endl;

            qDebug() << "WORKER SERVICE AFTER GET FILE PAYLOAD BEFORE EMIT";

            emit get_stream(s_datas);
            qDebug() << "WORKER SERVICE AFTER GET FILE PAYLOAD EMIT";
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

    if (s_datas.isValid() && s_datas.objsize() > 0)
    {
        qDebug() << "WORKER PROCESS BEFORE EMIT";
        emit push_payload(s_datas);
        qDebug() << "WORKER PROCESS AFTER EMIT";
    }

    //m_mutex->unlock();
}
