#include "get_payload.h"

Get_payload::~Get_payload()
{}


Get_payload::Get_payload(Nosql& a) : Worker(a)
{
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(watchdog ()), Qt::DirectConnection);
    timer->start (5000);
}


void Get_payload::init()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("worker" << "get_payload" << "action" << "register" << "pid" << QCoreApplication::applicationPid() << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}



void Get_payload::watchdog()
{
    QDateTime timestamp = QDateTime::currentDateTime();

    bo tracker = BSON("worker" << "get_payload" << "action" << "watchdog" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);
}



void Get_payload::s_job_receive(bson::bo data) {

    qDebug() << "Get_payload::s_job_receive";
    std::cout << "RECEIVE MESSAGE : " << data << std::endl;




    QDateTime timestamp = QDateTime::currentDateTime();
    QString s_timestamp = timestamp.toString("hhmmssz-ddMMyyyy");


    bo payload = BSON("action" << "torrent" << "format" << "binary" << "path" << "/tmp/" <<
                      "timestamp" << s_timestamp.toStdString() <<
                     "data" << data);


    std::cout << "PAYLOAD : " << payload << std::endl;





    timestamp = QDateTime::currentDateTime();
    bo tracker = BSON("worker" << "get_payload" << "action" << "payload" << "status" << "send" << "timestamp" << timestamp.toTime_t());
    emit return_tracker(tracker);

    //be created_at = payload["headers"]["created_at"];
    //cout << created_at.jsonString(TenGen) << endl;

    //be uuid = payload["headers"]["uuid"];

}
