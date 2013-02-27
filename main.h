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

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QDir>
#include <QxtCore/QxtCommandOptions>

#include "zeromq.h"
#include "service.h"
#include "process.h"
#include "ncw_global.h"


class NcwDaemon : public QObject
{
    Q_OBJECT

  public:
    NcwDaemon(QObject *parent = 0, const char *name = 0);
    ~NcwDaemon();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

  public slots:
    // Qt signal handlers.
    void handleSigHup();
    void handleSigTerm();

  private:
    static int sighupFd[2];
    static int sigtermFd[2];

    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;
};


#endif // MAIN_H
