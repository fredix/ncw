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



    // You should check if the JSON is well formed according what do you expect
    if (json.toMap().contains("field1")
            && json.toMap().contains("field2")
            && json.toMap().contains("field3")
            )
    {
         QString field1 = json.toMap()["field1"].toString();
         QString field2 = json.toMap()["field2"].toString();
         QString field3 = json.toMap()["field3"].toString();

         QString log = field1 + " " + field2 + " " + field3;

         handler_log->write(log.toAscii());
         handler_log->write("\n");
         handler_log->flush();

         sleep(2);

        //make your treatment and return a json with your response

         QString res = "{\"status\": \"ok\"}";

         //QString res = "{\"field1\": \"" + field1 + "\", \"field2\": \"" + field2 + "\", \"field3\": \"" + field3 + "\"}";

        writeStdout(res);
    }
    else
    {
        QString res = "{\"status\": \"error\"}";
        writeStdout(res);
    }


}




int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    


    QSettings settings("ncw", "worker_service");
    QxtCommandOptions options;

    options.add("debug", "show debug informations");
    options.alias("debug", "d");


    options.add("help", "show this help text");
    options.alias("help", "h");
    options.parse(QCoreApplication::arguments());



    if(options.count("help") || options.showUnrecognizedWarning()) {
        options.showUsage();
        return -1;
    }



    Io io;


    QThread *thread_handler = new QThread;
    NCWhandler ncwhandler;
    ncwhandler.moveToThread(thread_handler);
    thread_handler->start();

    QObject::connect(&io, SIGNAL(parseData(QString)), &ncwhandler, SLOT(push_payload(QString)), Qt::QueuedConnection);


    return a.exec();
}
