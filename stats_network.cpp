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



#include "stats_network.h"

Stats_network::Stats_network(Nosql& a, QString memcached_keycache) : Worker(a)
{
    cache_path.append(memcached_keycache).append(":views/report/network/");


    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


Stats_network::~Stats_network()
{}


void Stats_network::init(QString null)
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "name" << "stats_network" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}



void Stats_network::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("type" << "worker" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}


void Stats_network::s_job_receive(bson::bo payload) {


    bo bo_network_statistics;
    bob bob_network_statistics;
    bo bo_stats_network;
    bob bob_stats_network;


    qDebug() << "Stats_network::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;


    be created_at = payload["headers"]["created_at"];
    cout << created_at.jsonString(TenGen) << endl;

    be uuid = payload["headers"]["uuid"];




    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_network::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }


    std::cout << "Stats_network::s_job_receive, _id : " << host.getField("_id") << std::endl;


     //std::cout << "loadavg0 : " << r_hash msg.getField("loadavg0").toString() << std::endl;

    //std::cout << "loadavg0 : " << uuid.toString() << std::endl;


    bob_network_statistics << mongo::GENOID;
    bob_network_statistics << "host_id" << host_id;
    bob_network_statistics.append(created_at);
    bob_network_statistics << "rx_rate" << QString::fromStdString(payload["network"]["rx_rate"].valuestr()).toDouble()
                           << "tx_rate" << QString::fromStdString(payload["network"]["tx_rate"].valuestr()).toDouble();

    bo_network_statistics = bob_network_statistics.obj();

    nosql_.Insert("network_statistics", bo_network_statistics);


    qDebug() << "network stats inserted";




    std::cout << "stats_network? : " << host.hasField("stats_network") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_network") ? host.getFieldDotted("stats_network.counter").numberLong() + 1 : 1;
    double rx_rate = QString::fromStdString(payload["network"]["rx_rate"].valuestr()).toDouble();
    double tx_rate = QString::fromStdString(payload["network"]["tx_rate"].valuestr()).toDouble();

    double max_rx = (host.hasField("stats_network") && rx_rate < host.getFieldDotted("stats_network.max_rx").Double()) ? host.getFieldDotted("stats_network.max_rx").Double() : rx_rate;
    double all_rx = (host.hasField("stats_network")) ? host.getFieldDotted("stats_network.all_rx").Double() + rx_rate : rx_rate;

    double max_tx = (host.hasField("stats_network") && tx_rate < host.getFieldDotted("stats_network.max_tx").Double()) ? host.getFieldDotted("stats_network.max_tx").Double() : tx_rate;
    double all_tx = (host.hasField("stats_network")) ? host.getFieldDotted("stats_network.all_tx").Double() + tx_rate : tx_rate;


    //if (!host.hasField("stats_network")) bob_stats_network.append(msg.getField("created_at"));
    if (!host.hasField("stats_network")) bob_stats_network << "stats_network.created_at" << created_at;
    bob_stats_network << "stats_network.updated_at" << created_at;
    bob_stats_network << "stats_network.counter" << counter;
    bob_stats_network << "stats_network.rx_rate" << rx_rate;
    bob_stats_network << "stats_network.all_rx" << all_rx;
    bob_stats_network << "stats_network.tx_rate" << tx_rate;
    bob_stats_network << "stats_network.all_tx" << all_tx;
    bob_stats_network << "stats_network.average_rx" << all_rx / counter;
    bob_stats_network << "stats_network.average_tx" << all_tx / counter;
    bob_stats_network << "stats_network.max_rx" << max_rx;
    bob_stats_network << "stats_network.max_tx" << max_tx;

    //bo_stats_network = BSONObjBuilder().append("stats_network", bob_stats_network.obj()).obj();
    bo_stats_network = bob_stats_network.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_network);



    std::cout << "bo_stats_network : " << bo_stats_network.toString() << std::endl;

    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());



    QDateTime timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("type" << "worker" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}
