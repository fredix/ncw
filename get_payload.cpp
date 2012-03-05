#include "get_payload.h"

Get_payload::~Get_payload()
{}


Get_payload::Get_payload(Nosql& a) : Stats(a)
{}




void Get_payload::s_job_receive(bson::bo payload) {

    qDebug() << "Get_payload::s_job_receive";
    std::cout << "RECEIVE MESSAGE : " << payload << std::endl;


    //be created_at = payload["headers"]["created_at"];
    //cout << created_at.jsonString(TenGen) << endl;

    //be uuid = payload["headers"]["uuid"];


}
