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

Service::Service(QObject *parent) : QObject(parent)
{
    qDebug() << "Service::Service constructer";
    child_process = new QProcess();


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Service::~Service()
{
    delete(child_process);
}


void Service::init(QString child_exec, QString a_service_name)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    m_child_exec = child_exec;
    m_service_name = a_service_name;

    BSONObj tracker = BSON("type" << "service" << "name" << m_service_name.toStdString() << "command" << m_child_exec.toStdString() << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);

    qDebug() << "!!!!   EXEC PROCESS : " << child_exec;
    //child_process->start("/usr/bin/bttrack --bind 0.0.0.0 --port 6969 --dfile dstate");
    child_process->start(m_child_exec);
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