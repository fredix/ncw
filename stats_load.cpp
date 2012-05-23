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



#include "stats_load.h"

Stats_load::Stats_load(Nosql& a, QString memcached_keycache) : Worker(a)
{
    cache_path.append(memcached_keycache).append(":views/report/load/");


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Stats_load::~Stats_load()
{}


void Stats_load::init(QString null)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "name" << "stats_load" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Stats_load::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}



void Stats_load::s_job_receive(bson::bo payload) {

    bo bo_load_statistics;
    bob bob_load_statistics;
    bo bo_stats_load;
    bob bob_stats_load;

    qDebug() << "Stats_load::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;


    be created_at = payload["headers"]["created_at"];
    cout << created_at.jsonString(TenGen) << endl;

    be uuid = payload["headers"]["uuid"];




    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_load::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }

    std::cout << "Stats_load::s_job_receive, _id : " << host.getField("_id") << std::endl;


     //std::cout << "loadavg0 : " << r_hash msg.getField("loadavg0").toString() << std::endl;

    //std::cout << "loadavg0 : " << uuid.toString() << std::endl;


    long long counter = host.hasField("stats_load") ? host.getFieldDotted("stats_load.counter").numberLong() + 1 : 1;

    double loadavg0 = QString::fromStdString(payload["load"]["loadavg0"].valuestr()).toDouble();
    double loadavg1 = QString::fromStdString(payload["load"]["loadavg1"].valuestr()).toDouble();
    double loadavg2 = QString::fromStdString(payload["load"]["loadavg2"].valuestr()).toDouble();



    bob_load_statistics << mongo::GENOID;
    bob_load_statistics << "host_id" << host_id;
    bob_load_statistics.append(created_at);
    bob_load_statistics << "loadavg0" << loadavg0 << "loadavg1" << loadavg1 << "loadavg2" << loadavg2;
    bo_load_statistics = bob_load_statistics.obj();

    nosql_.Insert("load_statistics", bo_load_statistics);


    qDebug() << "load stats inserted";



    std::cout << "stats_load? : " << host.hasField("stats_load") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;




    double max_loadavg0 = (host.hasField("stats_load") && loadavg0 < host.getFieldDotted("stats_load.max_loadavg0").Double()) ? host.getFieldDotted("stats_load.max_loadavg0").Double() : loadavg0;
    double all_loadavg0 = (host.hasField("stats_load")) ? host.getFieldDotted("stats_load.all_loadavg0").Double() + loadavg0 : loadavg0;
    double max_loadavg1 = (host.hasField("stats_load") && loadavg1 < host.getFieldDotted("stats_load.max_loadavg1").Double()) ? host.getFieldDotted("stats_load.max_loadavg1").Double() : loadavg1;
    double all_loadavg1 = (host.hasField("stats_load")) ? host.getFieldDotted("stats_load.all_loadavg1").Double() + loadavg1 : loadavg1;
    double max_loadavg2 = (host.hasField("stats_load") && loadavg2 < host.getFieldDotted("stats_load.max_loadavg2").Double()) ? host.getFieldDotted("stats_load.max_loadavg2").Double() : loadavg2;
    double all_loadavg2 = (host.hasField("stats_load")) ? host.getFieldDotted("stats_load.all_loadavg2").Double() + loadavg2 : loadavg2;

    //if (!host.hasField("stats_load")) bob_stats_load.append(msg.getField("created_at"));
    if (!host.hasField("stats_load")) bob_stats_load << "stats_load.created_at" << created_at;
    bob_stats_load << "stats_load.updated_at" << created_at;
    bob_stats_load << "stats_load.counter" << counter;
    bob_stats_load << "stats_load.loadavg0" << loadavg0;
    bob_stats_load << "stats_load.loadavg1" << loadavg1;
    bob_stats_load << "stats_load.loadavg2" << loadavg2;
    bob_stats_load << "stats_load.all_loadavg0" << all_loadavg0;
    bob_stats_load << "stats_load.all_loadavg1" << all_loadavg1;
    bob_stats_load << "stats_load.all_loadavg2" << all_loadavg2;
    bob_stats_load << "stats_load.average_loadavg0" << all_loadavg0 / counter;
    bob_stats_load << "stats_load.average_loadavg1" << all_loadavg1 / counter;
    bob_stats_load << "stats_load.average_loadavg2" << all_loadavg2 / counter;
    bob_stats_load << "stats_load.max_loadavg0" << max_loadavg0;
    bob_stats_load << "stats_load.max_loadavg1" << max_loadavg1;
    bob_stats_load << "stats_load.max_loadavg2" << max_loadavg2;

    //bo_stats_load = BSONObjBuilder().append("stats_load", bob_stats_load.obj()).obj();

    bo_stats_load = bob_stats_load.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_load);



    std::cout << "bo_stats_load : " << bo_stats_load.toString() << std::endl;


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());



    QDateTime timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("type" << "worker" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);


}
