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



#include "stats_cpu.h"

Stats_cpu::Stats_cpu(Nosql& a, QString memcached_keycache) : Stats(a)
{    
    cache_path.append(memcached_keycache).append(":views/report/cpu/");
}


Stats_cpu::~Stats_cpu()
{}



void Stats_cpu::s_job_receive(std::string data) {


    bo bo_cpu_statistics;
    bob bob_cpu_statistics;
    bo bo_stats_cpu;
    bob bob_stats_cpu;

    qDebug() << "Stats_cpu::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;

    QString l_data = QString::fromStdString(data);


    //Deserializing
    QByteArray al = QByteArray::fromBase64(l_data.toAscii());

    Hash r_hash;

    QDataStream in(&al,QIODevice::ReadOnly);   // read the data serialized from the file
    in >> r_hash;

    qDebug() << "r_hash value: " << r_hash["xml"].toHash()["cpu_user"].toString();




    try {
        bo msg = mongo::fromjson(r_hash["bo"].toString().toStdString());



    be created_at = msg.getField("created_at");
    cout << created_at.jsonString(TenGen) << endl;

    be uuid = msg.getField("uuid");


    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_cpu::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }


    std::cout << "Stats_cpu::s_job_receive, _id : " << host.getField("_id") << std::endl;
    std::cout << "Stats_cpu::s_job_receive, architecture : " << host.getField("mem_total") << std::endl;


     //std::cout << "loadavg0 : " << r_hash msg.getField("loadavg0").toString() << std::endl;

    //std::cout << "loadavg0 : " << uuid.toString() << std::endl;


    bob_cpu_statistics << mongo::GENOID;
    bob_cpu_statistics << "host_id" << host_id;
    bob_cpu_statistics.append(msg.getField("created_at"));
    bob_cpu_statistics << "user" << r_hash["xml"].toHash()["user"].toDouble()
                       << "sys" << r_hash["xml"].toHash()["sys"].toDouble()
                       << "nice" << r_hash["xml"].toHash()["nice"].toDouble()
                       << "idle" << r_hash["xml"].toHash()["idle"].toDouble()
                       << "wait" << r_hash["xml"].toHash()["wait"].toDouble()
                       << "irq" << r_hash["xml"].toHash()["irq"].toDouble()
                       << "soft_irq" << r_hash["xml"].toHash()["soft_irq"].toDouble()
                       << "stolen" << r_hash["xml"].toHash()["stolen"].toDouble()
                       << "combined" << r_hash["xml"].toHash()["combined"].toDouble()
                       << "total" << r_hash["xml"].toHash()["total"].toDouble();
    bo_cpu_statistics = bob_cpu_statistics.obj();

    nosql_.Insert("cpu_statistics", bo_cpu_statistics);


    qDebug() << "cpu stats inserted";


    std::cout << "stats_cpu? : " << host.hasField("stats_cpu") << std::endl;

    //std::cout << "stats_cpu.number : " << host.getFieldDotted("stats_cpu.number").Int() + 1 << std::endl;


    long long counter = host.hasField("stats_cpu") ? host.getFieldDotted("stats_cpu.counter").numberLong() + 1 : 1;
    double combined = r_hash["xml"].toHash()["combined"].toDouble();

    double max_combined = (host.hasField("stats_cpu") && combined < host.getFieldDotted("stats_cpu.max_combined").Double()) ? host.getFieldDotted("stats_cpu.max_combined").Double() : combined;
    double all_combined = (host.hasField("stats_cpu")) ? host.getFieldDotted("stats_cpu.all_combined").Double() + combined : combined;

    // if (!host.hasField("stats_cpu")) bob_stats_cpu.append(msg.getField("created_at"));

    if (!host.hasField("stats_cpu")) bob_stats_cpu << "stats_cpu.created_at" << msg.getField("created_at");
    bob_stats_cpu << "stats_cpu.updated_at" << msg.getField("created_at");
    bob_stats_cpu << "stats_cpu.counter" << counter;
    bob_stats_cpu << "stats_cpu.combined" << combined;
    bob_stats_cpu << "stats_cpu.all_combined" << all_combined;
    bob_stats_cpu << "stats_cpu.average_combined" << all_combined / counter;
    bob_stats_cpu << "stats_cpu.max_combined" << max_combined;

    //bo_stats_cpu = BSONObjBuilder().append("stats_cpu", bob_stats_cpu.obj()).obj();

    bo_stats_cpu = bob_stats_cpu.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_cpu);



    std::cout << "bo_stats_cpu : " << bo_stats_cpu.toString() << std::endl;


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());


    //bo_stats_cpu = host.getField("stats_cpu").wrap();


    //std::cout << "host stats_cpu : " << host.getField("stats_cpu") << std::endl;

    //std::cout << "host stats_cpu : " << bo_stats_cpu.getField("combined").jsonString(TenGen, false) << std::endl;



    //bob_stats_cpu << "updated_at" << msg.getField("created_at").jsonString(TenGen, false)
      //               <<




/*if (endCondition(message)) {
       subscriptions.cancel(message.getDestination());
    }*/


    }
    catch(mongo::DBException &e) {
        std::cout << "Stats_cpu::s_job_receive, ERROR on serializing data : " << data << " , error : " << e.what() << std::endl;
        return;
    }


}
