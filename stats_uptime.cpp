/****************************************************************************
**   nodecast-worker is a bot worker, part of the backend of nodecast.net
**   Copyright (C) 2010-2011  Frédéric Logier <frederic@logier.org>
**
**   http://gitorious.org/nodecast/nodecast-worker
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



#include "stats_uptime.h"

Stats_uptime::Stats_uptime(Nosql& a, QString memcached_keycache) : Stats(a)
{    
    cache_path.append(memcached_keycache).append(":views/report/uptime/");
}


Stats_uptime::~Stats_uptime()
{}



void Stats_uptime::s_job_receive(std::string data) {

    bo bo_uptime_statistics;
    bob bob_uptime_statistics;
    bo bo_stats_uptime;
    bob bob_stats_uptime;

    qDebug() << "Stats_uptime::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;

    QString l_data = QString::fromStdString(data);


    //Deserializing
    QByteArray al = QByteArray::fromBase64(l_data.toAscii());


    Hash r_hash;



    QDataStream in(&al,QIODevice::ReadOnly);   // read the data serialized from the file
    in >> r_hash;

    qDebug() << "r_hash value: " << r_hash["xml"].toHash()["loadavg0"].toString();


    bo msg = mongo::fromjson(r_hash["bo"].toString().toStdString());

    be created_at = msg.getField("created_at");
    cout << created_at.jsonString(TenGen) << endl;

    be uuid = msg.getField("uuid");




    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");


    if (host.nFields() == 0)
    {
        std::cout << "Stats_uptime::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }


    std::cout << "Stats_uptime::s_job_receive, _id : " << host.getField("_id") << std::endl;


     //std::cout << "loadavg0 : " << r_hash msg.getField("loadavg0").toString() << std::endl;

    //std::cout << "loadavg0 : " << uuid.toString() << std::endl;


    bob_uptime_statistics << mongo::GENOID;
    bob_uptime_statistics << "host_id" << host_id;
    bob_uptime_statistics.append(msg.getField("created_at"));
    bob_uptime_statistics << "time" << r_hash["xml"].toHash()["time"].toDouble()
                          << "days" << r_hash["xml"].toHash()["days"].toString().toStdString();
    bo_uptime_statistics = bob_uptime_statistics.obj();

    nosql_.Insert("uptime_statistics", bo_uptime_statistics);


    qDebug() << "uptime stats inserted";



    std::cout << "stats_uptime? : " << host.hasField("stats_uptime") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_uptime") ? host.getFieldDotted("stats_uptime.counter").numberLong() + 1 : 1;
    double l_time = r_hash["xml"].toHash()["time"].toDouble();

    double max_time = (host.hasField("stats_uptime") && l_time < host.getFieldDotted("stats_uptime.max_time").Double()) ? host.getFieldDotted("stats_uptime.max_time").Double() : l_time;
    double all_time = (host.hasField("stats_uptime")) ? host.getFieldDotted("stats_uptime.all_time").Double() + l_time : l_time;


    if (!host.hasField("stats_uptime")) bob_stats_uptime << "stats_uptime.created_at" << msg.getField("created_at");
    bob_stats_uptime << "stats_uptime.updated_at" << msg.getField("created_at");
    bob_stats_uptime << "stats_uptime.counter" << counter;
    bob_stats_uptime << "stats_uptime.time" << l_time;
    bob_stats_uptime << "stats_uptime.all_time" << all_time;
    bob_stats_uptime << "stats_uptime.days" << r_hash["xml"].toHash()["days"].toString().toStdString();
    bob_stats_uptime << "stats_uptime.average" << all_time / counter;
    bob_stats_uptime << "stats_uptime.max_time" << max_time;

    //bo_stats_uptime = BSONObjBuilder().append("stats_uptime", bob_stats_uptime.obj()).obj();

    bo_stats_uptime = bob_stats_uptime.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_uptime);


    std::cout << "bo_stats_uptime : " << bo_stats_uptime.toString() << std::endl;

    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());



/*if (endCondition(message)) {
       subscriptions.cancel(message.getDestination());
    }*/


}
