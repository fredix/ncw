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

#include <QCoreApplication>
#include "main.h"



Io::~Io()
{
    io_log->close();
}

Io::Io() : QObject()
{
    input  = new QTextStream( stdin,  QIODevice::ReadOnly );

    // Demand notification when there is data to be read from stdin
    notifier = new QSocketNotifier( STDIN_FILENO, QSocketNotifier::Read );
    connect(notifier, SIGNAL(activated(int)), this, SLOT(readStdin()));


    io_log = new QFile("/tmp/worker_service_io.log");
    if (!io_log->open(QIODevice::Append | QIODevice::Text))
            return;


    //*cout << prompt;
}

void Io::readStdin()
{
    //notifier->setEnabled(false);


//    qDebug() << "READ STDIN";
    // Read the data
    QString line = input->readLine();

    io_log->write(line.toAscii());
    io_log->write("\n");
    io_log->flush();

    //writeStdout("LINE : " + line);
    //if (!line.isNull() && !line.isEmpty())
    //{
    // Parse received data

    emit parseData(line);


    //}

    //notifier->setEnabled(true);
}


NCWhandler::~NCWhandler()
{
    handler_log->close();
}

NCWhandler::NCWhandler() : QObject()
{


    output = new QTextStream( stdout, QIODevice::WriteOnly );

    handler_log = new QFile("/tmp/worker_service_handler.log");
     if (!handler_log->open(QIODevice::Append | QIODevice::Text))
             return;
}


// Handler send JSON to NCS through the STDOUT
void NCWhandler::writeStdout(QString s)
{
    (*output) << s;
    //while (!(*output).atEnd()) {};
    (*output).flush();
}


// Handler receive JSON from NCS through the STDIN
void NCWhandler::push_payload(QString s)
{


    QVariant json = QxtJSON::parse(s);


         handler_log->write(json.toString().toAscii());
         handler_log->write("\n");
         handler_log->flush();


         emit emit_payload(json);



        //make your treatment and return a json with your response

         QString res = "{\"status\": \"ok\"}";


        writeStdout(res);

    //    QString res = "{\"status\": \"error\"}";
        writeStdout(res);


}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    email_params emailparams;


    QSettings settings("ncw", "mail_service");
    QxtCommandOptions options;

    options.add("debug", "show debug informations");
    options.alias("debug", "d");



    options.add("smtp-hostname", "set the smtp hostname", QxtCommandOptions::Required);
    options.alias("smtp-hostname", "sph");

    options.add("smtp-username", "set the smtp username", QxtCommandOptions::Required);
    options.alias("smtp-username", "spu");

    options.add("smtp-password", "set the smtp password", QxtCommandOptions::Required);
    options.alias("smtp-password", "spp");


    options.add("smtp-sender", "set the smtp sender", QxtCommandOptions::Required);
    options.alias("smtp-sender", "sps");


    options.add("smtp-recipient", "set the smtp recipient", QxtCommandOptions::Required);
    options.alias("smtp-recipient", "spr");




    options.add("help", "show this help text");
    options.alias("help", "h");
    options.parse(QCoreApplication::arguments());



    if(options.count("help") || options.showUnrecognizedWarning()) {
        options.showUsage();
        return -1;
    }




    if(options.count("smtp-hostname")) {
        emailparams.smtp_hostname = options.value("smtp-hostname").toString();
        settings.setValue("smtp-hostname", emailparams.smtp_hostname);
    }
    else if(settings.contains("smtp-hostname"))
    {
        emailparams.smtp_hostname = settings.value("smtp-hostname").toString();
    }
    else {
        std::cout << "ncs: --smtp-hostname requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("smtp-username")) {
        emailparams.smtp_username = options.value("smtp-username").toString();
        settings.setValue("smtp-username", emailparams.smtp_username);
    }
    else if(settings.contains("smtp-username"))
    {
        emailparams.smtp_username = settings.value("smtp-username").toString();
    }
    else {
        std::cout << "ncs: --smtp-username requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }


    if(options.count("smtp-password")) {
        emailparams.smtp_password = options.value("smtp-password").toString();
        settings.setValue("smtp-password", emailparams.smtp_password);
    }
    else if(settings.contains("smtp-password"))
    {
        emailparams.smtp_password = settings.value("smtp-password").toString();
    }
    else {
        std::cout << "ncs: --smtp-password requires a parameter" << std::endl;
        options.showUsage();
        return -1;
    }




    Io io;


    QThread *thread_email = new QThread;
    Email email(emailparams);


    email.moveToThread(thread_email);
    thread_email->start();

    //QObject::connect(&io, SIGNAL(parseData(QString)), &email, SLOT(sendEmail(QString)), Qt::QueuedConnection);




    QThread *thread_handler = new QThread;
    NCWhandler ncwhandler;
    ncwhandler.moveToThread(thread_handler);
    thread_handler->start();

    QObject::connect(&io, SIGNAL(parseData(QString)), &ncwhandler, SLOT(push_payload(QString)), Qt::QueuedConnection);
    QObject::connect(&ncwhandler, SIGNAL(emit_payload(QVariant)), &email, SLOT(sendEmail(QVariant)), Qt::QueuedConnection);

    return a.exec();
}
