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


#include "stats_memory.h"

Stats_memory::Stats_memory(Nosql& a, QString memcached_keycache) : Stats(a)
{
    cache_path.append(memcached_keycache).append(":views/report/memory/");
}


Stats_memory::~Stats_memory()
{}



void Stats_memory::s_job_receive(std::string data) {


    bo bo_memory_statistics;
    bob bob_memory_statistics;
    bo bo_stats_memory;
    bob bob_stats_memory;


    qDebug() << "Stats_memory::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;

    QString l_data = QString::fromStdString(data);


    //Deserializing
    QByteArray al = QByteArray::fromBase64(l_data.toAscii());


    Hash r_hash;



    QDataStream in(&al,QIODevice::ReadOnly);   // read the data serialized from the file
    in >> r_hash;

    qDebug() << "r_hash value: " << r_hash["xml"].toHash()["mem_used"].toString();





    bo msg = mongo::fromjson(r_hash["bo"].toString().toStdString());

    be created_at = msg.getField("created_at");
    std::cout << created_at.jsonString(TenGen) << std::endl;

    be uuid = msg.getField("uuid");




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
    bob_memory_statistics.append(msg.getField("created_at"));
    bob_memory_statistics << "mem_used" << r_hash["xml"].toHash()["mem_used"].toDouble()
                       << "mem_free" << r_hash["xml"].toHash()["mem_free"].toDouble()
                       << "mem_actual_free" << r_hash["xml"].toHash()["mem_actual_free"].toDouble()
                       << "mem_actual_used" << r_hash["xml"].toHash()["mem_actual_used"].toDouble()
                       << "mem_actual_free_percent" << r_hash["xml"].toHash()["mem_actual_free_percent"].toDouble()
                       << "mem_actual_used_percent" << r_hash["xml"].toHash()["mem_actual_used_percent"].toDouble()
                       << "swap_total" << r_hash["xml"].toHash()["swap_total"].toDouble()
                       << "swap_used" << r_hash["xml"].toHash()["swap_used"].toDouble()
                       << "swap_free" << r_hash["xml"].toHash()["swap_free"].toDouble()
                       << "swap_page_in" << r_hash["xml"].toHash()["swap_page_in"].toDouble()
                       << "swap_page_out" << r_hash["xml"].toHash()["swap_page_out"].toDouble();
    bo_memory_statistics = bob_memory_statistics.obj();

    nosql_.Insert("memory_statistics", bo_memory_statistics);


    qDebug() << "memory stats inserted";





    std::cout << "stats_memory? : " << host.hasField("stats_memory") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_memory") ? host.getFieldDotted("stats_memory.counter").numberLong() + 1 : 1;
    double mem_used = r_hash["xml"].toHash()["mem_used"].toDouble();

    double max_mem_used = (host.hasField("stats_memory") && mem_used < host.getFieldDotted("stats_memory.max_mem_used").Double()) ? host.getFieldDotted("stats_memory.max_mem_used").Double() : mem_used;
    double all_mem_used = (host.hasField("stats_memory")) ? host.getFieldDotted("stats_memory.all_mem_used").Double() + mem_used : mem_used;

    //if (!host.hasField("stats_memory")) bob_stats_memory.append(msg.getField("created_at"));
    if (!host.hasField("stats_memory")) bob_stats_memory << "stats_memory.created_at" << msg.getField("created_at");
    bob_stats_memory << "stats_memory.updated_at" << msg.getField("created_at");
    bob_stats_memory << "stats_memory.counter" << counter;
    bob_stats_memory << "stats_memory.mem_used" << mem_used;
    bob_stats_memory << "stats_memory.all_mem_used" << all_mem_used;
    bob_stats_memory << "stats_memory.average_mem_used" << all_mem_used / counter;
    bob_stats_memory << "stats_memory.max_mem_used" << max_mem_used;
    bob_stats_memory << "stats_memory.swap_total" << r_hash["xml"].toHash()["swap_total"].toDouble();

    //bo_stats_memory = BSONObjBuilder().append("stats_memory", bob_stats_memory.obj()).obj();
    bo_stats_memory = bob_stats_memory.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_memory);



    std::cout << "bo_stats_memory : " << bo_stats_memory.toString() << std::endl;


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());








/*if (endCondition(message)) {
       subscriptions.cancel(message.getDestination());
    }*/


}
