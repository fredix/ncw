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

#include "service.h"

Service::Service(Nosql& a, QObject *parent) : nosql_(a), QObject(parent)
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


void Service::init(QString child_exec, QString service_name)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "service" << "name" << service_name.toStdString() << "command" << child_exec.toStdString() << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);

    qDebug() << "!!!!   EXEC PROCESS : " << child_exec;
    //child_process->start("/usr/bin/bttrack --bind 0.0.0.0 --port 6969 --dfile dstate");
    child_process->start(child_exec);
    qDebug() << "PID : " << child_process->pid();
}


void Service::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    if (child_process->pid() != 0)
    {

        bo tracker = BSON("type" << "service" << "action" << "watchdog" << "child_pid" << child_process->pid() << "timestamp" << timestamp.toTime_t());

        emit return_tracker(tracker);
    }
    else {

        bo tracker = BSON("type" << "service" << "action" << "watchdog" << "child_pid" << child_process->pid() << "timestamp" << timestamp.toTime_t());

        emit return_tracker(tracker);
        QCoreApplication::exit(1);
    }
}
