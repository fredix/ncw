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

#ifndef PROCESS_H
#define PROCESS_H


#include "worker.h"


class Process : public Worker
{
public:
    Process(ncw_params a_ncw);
    ~Process();
    void init();

private:
    QTimer *timer;
    QProcess *process;
    QString m_child_exec;
    QString m_process_name;
    QString m_node_uuid;
    QString m_node_password;
    QString m_output;
    int m_exitcode;
    int m_exitstatus;
    ncw_params m_ncw;

private slots:
    void watchdog();
    void process_finished(int exitCode, QProcess::ExitStatus exitStatus);

public slots:
    void s_job_receive(bson::bo data);

};

#endif // PROCESS_H
