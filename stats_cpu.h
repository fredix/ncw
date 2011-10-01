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


#ifndef STATS_CPU_H
#define STATS_CPU_H

#include "stats.h"

class Stats_cpu : public Stats
{
public:
    //Stats_process(QObject *parent = 0);
      Stats_cpu(Nosql& a, QString memcached_keycache);
      //Stats_process();
      ~Stats_cpu();


    QString cache_path;
    //bo bo_process_statistics;
    //list<bo> lbo_processus;
    //bob bob_process_statistics;
    //bo host;

protected:
    //Nosql &nosql_;

private:
    // QDomDocument m_xml_datas;

signals:

public slots:
    void s_job_receive(std::string data);
};


#endif // STATS_CPU_H
