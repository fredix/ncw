/****************************************************************************
**   ncw is the nodecast worker, client of the nodecast server
**   Copyright (C) 2010-2013  Frédéric Logier <frederic@logier.org>
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


#include <QSettings>
#include <QxtCore/QxtCommandOptions>
#include <qstring.h>
#include <qtextstream.h>
#include <qsocketnotifier.h>
#include <QxtJSON>
//#include <QxtNetwork>
#include <QDateTime>
#include <iostream>
#include <unistd.h>
#include <QFile>
#include <QThread>
#include <QTextStream>
#include "email.h"


class Io : public QObject
{
    Q_OBJECT

public:
    Io();
    ~Io();

private slots:
    void readStdin();

private:
    QFile *io_log;
    QSocketNotifier* notifier;
    QTextStream* input;

signals:
    void parseData(QString s);

};



class NCWhandler : public QObject
{
    Q_OBJECT

public:
    NCWhandler();
    ~NCWhandler();

public slots:
    void push_payload(QString s);
    void writeStdout(QString s);

signals:
    void emit_payload(QVariant payload);

private:
    QFile *handler_log;
    QTextStream* output;
};


#endif // MAIN_H
