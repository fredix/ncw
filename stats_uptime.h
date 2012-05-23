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


#ifndef STATS_UPTIME_H
#define STATS_UPTIME_H

#include "worker.h"

class Stats_uptime : public Worker
{
public:
    //Stats_process(QObject *parent = 0);
      Stats_uptime(Nosql& a, QString memcached_keycache);
      //Stats_process();
      ~Stats_uptime();
      void init(QString null);


    QString cache_path;
    //bo bo_process_statistics;
    //list<bo> lbo_processus;
    //bob bob_process_statistics;
    //bo host;

protected:
    //Nosql &nosql_;

private:
    QTimer *timer;

private slots:
    void watchdog();


public slots:
    void s_job_receive(bson::bo data);
};


#endif // STATS_UPTIME_H
