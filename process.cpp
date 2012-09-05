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

#include "process.h"

Process::Process() : Worker()
{
    process = new QProcess();
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(process_finished(int,QProcess::ExitStatus)));


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Process::~Process()
{
    delete(process);
}


//void Process::init(QString child_exec, QString a_process_name)
void Process::init(ncw_params ncw)
{
    QDateTime timestamp = QDateTime::currentDateTime();    

    m_child_exec = ncw.child_exec;
    m_process_name = ncw.worker_name;
    m_node_uuid = ncw.node_uuid;
    m_node_password = ncw.node_password;

    BSONObj tracker = BSON("type" << "worker" << "name" << m_process_name.toStdString() << "command" << m_child_exec.toStdString() << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Process::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    BSONObj tracker = BSON("type" << "worker" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}




void Process::s_job_receive(bson::bo data) {

    qDebug() << "process::s_job_receive";

    BSONElement r_datas = data.getField("datas");
    BSONElement session_uuid = data.getField("session_uuid");
    //data.getField("step_id").Obj().getObjectID(step_id);

    //std::cout << "step_id " << step_id << std::endl;
    //be step_id;
    //data.getObjectID (step_id);

    std::cout << "RECEIVE MESSAGE : " << data << std::endl;


    QString param;

    QDateTime timestamp = QDateTime::currentDateTime();

    BSONObjBuilder b_tracker;
    b_tracker << "type" << "worker";
    b_tracker.append(session_uuid);
    b_tracker << "name" << m_process_name.toStdString() << "action" << "receive" << "timestamp" << timestamp.toTime_t();

    BSONObj tracker = b_tracker.obj();
    emit push_payload(tracker.copy());

    param.append(" ").append(QString::fromStdString(r_datas.str()));

    qDebug() << "!!!!   EXEC PROCESS : " << m_child_exec;
    process->start(m_child_exec + param);
    process->waitForFinished(-1);

    timestamp = QDateTime::currentDateTime();
    //tracker = BSON("type" << "worker" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    //emit return_tracker(tracker);

    BSONObjBuilder b_datas;
    b_datas << "type" << "worker";
    b_datas.append(session_uuid);
    b_datas << "name" << m_process_name.toStdString() << "action" << "terminate" << "timestamp" << timestamp.toTime_t() << "datas" << m_output.toStdString() << "exitcode" << m_exitcode << "exitstatus" << m_exitstatus;


    BSONObj s_datas = b_datas.obj();

    std::cout << "s_datas : " << s_datas << std::endl;

    qDebug() << "WORKER PROCESS BEFORE EMIT";
    emit push_payload(s_datas.copy());
    qDebug() << "WORKER PROCESS AFTER EMIT";
}

void Process::process_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process::process_finished exitcode : " << exitCode << " , exitStatus : " << exitStatus;
    m_output = process->readAllStandardOutput ();
    m_exitcode = exitCode;
    m_exitstatus = exitStatus;
    qDebug() <<  m_output;
}
