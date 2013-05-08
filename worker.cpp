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


#include "worker.h"

Worker::Worker(QObject *parent) : QObject(parent)
{}


Worker::~Worker()
{}



void Worker::init()
{}

void Worker::launch()
{}


void Worker::watchdog()
{}


void Worker::readyReadStandardOutput()
{}


void Worker::process_write(qint64 val)
{}

void Worker::s_job_receive(bson::bo data)
{
    qDebug() << "Worker::s_mother_job_receive";
}


void Worker::get_pubsub(string data)
{
    qDebug() << "Worker::mother_get_pubsub";
}


void Worker::process_finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Worker::process_finished exitcode : " << exitCode << " , exitStatus : " << exitStatus;
}
