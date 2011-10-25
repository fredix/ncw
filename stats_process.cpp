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


#include "stats_process.h"

//Stats_process::Stats_process(QObject *parent) :


Stats_process::Stats_process(Nosql& a, QString memcached_keycache) : Stats(a)
{
    cache_path.append(memcached_keycache).append(":views/report/processus/");
}

/*
Stats_process::Stats_process()
{}
*/

Stats_process::~Stats_process()
{}



void Stats_process::s_job_receive(bson::bo payload) {

    qDebug() << "Stats_process::s_job_receive";
    std::cout << "RECEIVE MESSAGE" << std::endl;

    bo bo_process_statistics;
    bob bob_process_statistics;
    bo bo_stats_process;
    bob bob_stats_process;


    be uuid = payload["headers"]["uuid"];
    be created_at = payload["headers"]["created_at"];
    be gfs_id = payload["headers"]["_id"];

    cout << uuid.toString() << endl;
    cout << created_at.jsonString(TenGen) << endl;
    cout << created_at.jsonString(JS) << endl;
    cout << created_at.jsonString(Strict) << endl;
    cout << gfs_id.jsonString(TenGen) << endl;

    bo host = nosql_.Find("hosts", uuid.wrap());
    be host_id = host.getField("_id");

    if (host.nFields() == 0)
    {
        std::cout << "Stats_process::s_job_receive, HOST NOT FOUND" << std::endl;
        return;
    }

    std::cout << "Stats_process::s_job_receive, _id : " << host.getField("_id") << std::endl;


    bo l_json_datas = nosql_.ExtractJSON(gfs_id);


    bob_process_statistics << mongo::GENOID;

    qDebug() << "GENOID bob_process_statistics";


    bob_process_statistics.append(created_at);


    bob_process_statistics << "host_id" << host_id;


    qDebug() << "host_id bob_process_statistics";


    bob_process_statistics <<  "process_number" <<  l_json_datas["process"]["process_number"];


    qDebug() << "process_number bob_process_statistics";


    bob_process_statistics << "processus" << l_json_datas["process"]["processus"];


    qDebug() << "create bob_process_statistics";


    bo_process_statistics = bob_process_statistics.obj();

    qDebug() << "before insert process_statistics";

    nosql_.Insert("process_statistics", bo_process_statistics);


    qDebug() << "process stats inserted";


    std::cout << "stats_process? : " << host.hasField("stats_process") << std::endl;

    long long counter = host.hasField("stats_process") ? host.getFieldDotted("stats_process.counter").numberLong() + 1 : 1;

    //if (!host.hasField("stats_process")) bob_stats_process.append(msg.getField("created_at"));
    if (!host.hasField("stats_process")) bob_stats_process << "stats_process.created_at" << created_at;
    bob_stats_process << "stats_process.updated_at" << created_at;
    bob_stats_process << "stats_process.counter" << counter;
    bob_stats_process << "stats_process.process_number" << l_json_datas["process"]["process_number"];

    //bo_stats_process = BSONObjBuilder().append("stats_process", bob_stats_process.obj()).obj();
    bo_stats_process = bob_stats_process.obj();
    nosql_.Update("hosts", host_id.wrap(), bo_stats_process);


    std::cout << "bo_stats_process : " << bo_stats_process.toString() << std::endl;


    qDebug() << cache_path.toStdString().c_str();


    std::cout << "cache path : " << cache_path.toStdString() << std::endl;
    emit delete_cache(cache_path + uuid.String().data());
}
