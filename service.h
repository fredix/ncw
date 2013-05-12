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

#include "worker.h"
#include <QFile>

class Service : public Worker
{    
public:
    Service(zmq::context_t *a_context, ncw_params a_ncw);
    ~Service();

private:
    void received_file(string filename, bool status);

    zmq::context_t *m_context;
    zmq::socket_t *z_worker;
    zmq::message_t *z_message;
    QMutex *m_mutex;
    QTimer *timer;
    QProcess *child_process;
    QString m_child_exec;
    QString m_service_name;
    QString m_session_uuid;
    QString m_node_uuid;
    QString m_node_password;
    ncw_params m_ncw;

private slots:
     void watchdog();
     void readyReadStandardOutput();

public slots:
     void init();
     void launch();
     void s_job_receive(bson::bo data);
     void get_pubsub(QString data);
};

#endif // SERVICE_H
