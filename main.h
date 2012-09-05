/****************************************************************************
**   ncw is the nodecast worker, client of the nodecast server
**   Copyright (C) 2010-2012  Frédéric Logier <frederic@logier.org>
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


#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <signal.h>

#include <QDebug>
//#include <QDomDocument>
#include <QxtCore/QxtCommandOptions>

#include "zeromq.h"
#include "service.h"
#include "process.h"
#include "ncw_global.h"



enum WorkerType {
    WSERVICE=1,
    WPROCESS=2
};

typedef QMap<QString, WorkerType> StringToEnumMap;



class Zworker : public QObject
{
    Q_OBJECT
public:
    Zworker();
    ~Zworker();
    //void Init(QString worker_type, QString worker_name, QString child_exec);
    void Init(ncw_params ncw);
    Zeromq *zeromq;
    Worker *worker;
    Process *process;
    Service *service;
};


#endif // MAIN_H
