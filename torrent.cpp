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

#include "torrent.h"

Torrent::Torrent(Nosql& a) : Worker(a)
{

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Torrent::~Torrent()
{}




void Torrent::init(QString null)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "name" << "torrent" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Torrent::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}




void Torrent::s_job_receive(bson::bo data) {

    qDebug() << "process::s_job_receive";
    std::cout << "RECEIVE MESSAGE : " << data << std::endl;

    QDateTime timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("worker" << "torrent" << "action" << "payload" << "status" << "receive" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);





    timestamp = QDateTime::currentDateTime();
    tracker = BSON("type" << "worker" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);


}
