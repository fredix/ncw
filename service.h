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

#ifndef SERVICE_H
#define SERVICE_H


#include <QCoreApplication>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include "worker.h"


class Service : public QObject
{
    Q_OBJECT
public:
    Service(QObject *parent = 0);
    ~Service();
    void init(QString child_exec, QString service_name);

private:
    QTimer *timer;
    QProcess *child_process;

private slots:
     void watchdog();

signals:
    void return_tracker(bson::bo data);
};

#endif // SERVICE_H
