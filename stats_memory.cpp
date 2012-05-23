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


#include "stats_memory.h"

Stats_memory::Stats_memory(Nosql& a, QString memcached_keycache) : Worker(a)
{
    cache_path.append(memcached_keycache).append(":views/report/memory/");


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Stats_memory::~Stats_memory()
{}


void Stats_memory::init(QString null)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "name" << "stats_memory" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}



void Stats_memory::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Stats_memory::s_job_receive(bson::bo payload) {


    bo bo_memory_statistics;
    bob bob_memory_statistics;
    bo bo_stats_memory;
    bob bob_stats_memory;


    qDebug() << "Stats_memory::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;


    be created_at = payload["headers"]["created_at"];
    std::cout << created_at.jsonString(TenGen) << std::endl;

    be uuid = payload["headers"]["uuid"];



    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_memory::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }



    std::cout << "Stats_memory::s_job_receive, _id : " << host.getField("_id") << std::endl;


     //std::cout << "loadavg0 : " << r_hash msg.getField("loadavg0").toString() << std::endl;

    //std::cout << "loadavg0 : " << uuid.toString() << std::endl;


    bob_memory_statistics << mongo::GENOID;
    bob_memory_statistics << "host_id" << host_id;
    bob_memory_statistics.append(created_at);
    bob_memory_statistics << "mem_used" << QString::fromStdString(payload["memory"]["mem_used"].valuestr()).toDouble()
                       << "mem_free" << QString::fromStdString(payload["memory"]["mem_free"].valuestr()).toDouble()
                       << "mem_actual_free" << QString::fromStdString(payload["memory"]["mem_actual_free"].valuestr()).toDouble()
                       << "mem_actual_used" << QString::fromStdString(payload["memory"]["mem_actual_used"].valuestr()).toDouble()
                       << "mem_actual_free_percent" << QString::fromStdString(payload["memory"]["mem_actual_free_percent"].valuestr()).toDouble()
                       << "mem_actual_used_percent" << QString::fromStdString(payload["memory"]["mem_actual_used_percent"].valuestr()).toDouble()
                       << "swap_total" << QString::fromStdString(payload["memory"]["swap_total"].valuestr()).toDouble()
                       << "swap_used" << QString::fromStdString(payload["memory"]["swap_used"].valuestr()).toDouble()
                       << "swap_free" << QString::fromStdString(payload["memory"]["swap_free"].valuestr()).toDouble()
                       << "swap_page_in" << QString::fromStdString(payload["memory"]["swap_page_in"].valuestr()).toDouble()
                       << "swap_page_out" << QString::fromStdString(payload["memory"]["swap_page_out"].valuestr()).toDouble();
    bo_memory_statistics = bob_memory_statistics.obj();

    nosql_.Insert("memory_statistics", bo_memory_statistics);


    qDebug() << "memory stats inserted";





    std::cout << "stats_memory? : " << host.hasField("stats_memory") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_memory") ? host.getFieldDotted("stats_memory.counter").numberLong() + 1 : 1;
    double mem_used = QString::fromStdString(payload["memory"]["mem_used"].valuestr()).toDouble();

    double max_mem_used = (host.hasField("stats_memory") && mem_used < host.getFieldDotted("stats_memory.max_mem_used").Double()) ? host.getFieldDotted("stats_memory.max_mem_used").Double() : mem_used;
    double all_mem_used = (host.hasField("stats_memory")) ? host.getFieldDotted("stats_memory.all_mem_used").Double() + mem_used : mem_used;

    //if (!host.hasField("stats_memory")) bob_stats_memory.append(msg.getField("created_at"));
    if (!host.hasField("stats_memory")) bob_stats_memory << "stats_memory.created_at" << created_at;
    bob_stats_memory << "stats_memory.updated_at" << created_at;
    bob_stats_memory << "stats_memory.counter" << counter;
    bob_stats_memory << "stats_memory.mem_used" << mem_used;
    bob_stats_memory << "stats_memory.all_mem_used" << all_mem_used;
    bob_stats_memory << "stats_memory.average_mem_used" << all_mem_used / counter;
    bob_stats_memory << "stats_memory.max_mem_used" << max_mem_used;
    bob_stats_memory << "stats_memory.swap_total" << QString::fromStdString(payload["memory"]["swap_total"].valuestr()).toDouble();

    //bo_stats_memory = BSONObjBuilder().append("stats_memory", bob_stats_memory.obj()).obj();
    bo_stats_memory = bob_stats_memory.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_memory);



    std::cout << "bo_stats_memory : " << bo_stats_memory.toString() << std::endl;


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());


    QDateTime timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("type" << "worker" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}
