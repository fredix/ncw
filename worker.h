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


#ifndef STATS_H
#define STATS_H
/*
#include <QDebug>
#include <QFile>
#include <QDomDocument>
#include <QObject>

#include "mongodb/client/gridfs.h"
#include "mongodb/bson/bson.h"
#include "mongodb/client/dbclient.h"
*/
#include "libmemcached/memcached.hpp"

#include "nosql.h"

using namespace mongo;
using namespace bson;



class Stats : public QObject
{
    Q_OBJECT
public:
//    explicit stats(QObject *parent = 0);
    Stats(Nosql& a, QObject *parent = 0);
 //   Stats();
    ~Stats();

protected:
     Nosql &nosql_;

signals:
    void delete_cache(QString pub_uuid);    
    void return_payload(bson::bo data);

public slots:
     virtual void s_job_receive(bson::bo data)=0;
};

#endif // STATS_H
