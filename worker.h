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


#ifndef WORKER_H
#define WORKER_H

#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"

#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QMutex>
#include "ncw_global.h"

using namespace mongo;
using namespace bson;



class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(QObject *parent = 0);
    ~Worker();
    virtual void init()=0;

private slots:    
    virtual void watchdog()=0;
    virtual void process_finished(int exitCode, QProcess::ExitStatus exitStatus);
    virtual void readyReadStandardOutput();


signals:
    void push_payload(bson::bo data);
    void return_tracker(bson::bo data);
    void get_stream(bson::bo data, string filename, bool *status);

public slots:
    virtual void launch();
    virtual void s_job_receive(bson::bo data)=0;
    virtual void get_pubsub(string data);
};

#endif // WORKER_H
