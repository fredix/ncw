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



#include "stats_cpu.h"

Stats_cpu::Stats_cpu(Nosql& a, QString memcached_keycache) : Worker(a)
{    
    cache_path.append(memcached_keycache).append(":views/report/cpu/");

    qDebug() << "Dispatcher::Dispatcher constructer";


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Stats_cpu::~Stats_cpu()
{}


void Stats_cpu::init(QString null)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "name" << "stats_cpu" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Stats_cpu::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}

void Stats_cpu::s_job_receive(bson::bo payload) {

    bo bo_cpu_statistics;
    bob bob_cpu_statistics;
    bo bo_stats_cpu;
    bob bob_stats_cpu;

    qDebug() << "Stats_cpu::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;



    be created_at = payload["headers"]["created_at"];
    cout << created_at.jsonString(TenGen) << endl;

    be uuid = payload["headers"]["uuid"];


    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_cpu::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }
    std::cout << "Stats_cpu::s_job_receive, payload : " << mongo::BSONType(payload["cpu_usage"]["total"].type()) << std::endl;


    bob_cpu_statistics << mongo::GENOID;
    bob_cpu_statistics << "host_id" << host_id;
    bob_cpu_statistics.append(created_at);
    bob_cpu_statistics << "user" << QString::fromStdString(payload["cpu_usage"]["user"].valuestr()).toDouble()
                       << "sys" << QString::fromStdString(payload["cpu_usage"]["sys"].valuestr()).toDouble()
                       << "nice" << QString::fromStdString(payload["cpu_usage"]["nice"].valuestr()).toDouble()
                       << "idle" << QString::fromStdString(payload["cpu_usage"]["idle"].valuestr()).toDouble()
                       << "wait" << QString::fromStdString(payload["cpu_usage"]["wait"].valuestr()).toDouble()
                       << "irq" << QString::fromStdString(payload["cpu_usage"]["irq"].valuestr()).toDouble()
                       << "soft_irq" << QString::fromStdString(payload["cpu_usage"]["soft_irq"].valuestr()).toDouble()
                       << "stolen" << QString::fromStdString(payload["cpu_usage"]["stolen"].valuestr()).toDouble()
                       << "combined" << QString::fromStdString(payload["cpu_usage"]["combined"].valuestr()).toDouble()
                       << "total" << QString::fromStdString(payload["cpu_usage"]["total"].valuestr()).toDouble();
    bo_cpu_statistics = bob_cpu_statistics.obj();

    nosql_.Insert("cpu_statistics", bo_cpu_statistics);


    qDebug() << "cpu stats inserted";


    std::cout << "stats_cpu? : " << host.hasField("stats_cpu") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_cpu") ? host.getFieldDotted("stats_cpu.counter").numberLong() + 1 : 1;
    double combined = payload["cpu_usage"]["combined"].numberDouble();


    double max_combined = (host.hasField("stats_cpu") && combined < host.getFieldDotted("stats_cpu.max_combined").Double()) ? host.getFieldDotted("stats_cpu.max_combined").Double() : combined;
    double all_combined = (host.hasField("stats_cpu")) ? host.getFieldDotted("stats_cpu.all_combined").Double() + combined : combined;

    // if (!host.hasField("stats_cpu")) bob_stats_cpu.append(msg.getField("created_at"));

    if (!host.hasField("stats_cpu")) bob_stats_cpu << "stats_cpu.created_at" << created_at;
    bob_stats_cpu << "stats_cpu.updated_at" << created_at;
    bob_stats_cpu << "stats_cpu.counter" << counter;
    bob_stats_cpu << "stats_cpu.combined" << combined;
    bob_stats_cpu << "stats_cpu.all_combined" << all_combined;
    bob_stats_cpu << "stats_cpu.average_combined" << all_combined / counter;
    bob_stats_cpu << "stats_cpu.max_combined" << max_combined;


    bo_stats_cpu = bob_stats_cpu.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_cpu);


    std::cout << "bo_stats_cpu : " << bo_stats_cpu.toString() << std::endl;


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());


    QDateTime timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}
