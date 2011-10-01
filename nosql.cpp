/****************************************************************************
**   nodecast-worker is a bot worker, part of the backend of nodecast.net
**   Copyright (C) 2010-2011  Frédéric Logier <frederic@logier.org>
**
**   http://gitorious.org/nodecast/nodecast-worker
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


#include "nosql.h"


Nosql::Nosql()
{
    qDebug() << "Nosql construct";
}


Nosql::Nosql(QString a_server, QString a_database) : m_server(a_server), m_database(a_database)
{
    qDebug() << "Nosql construct param";

    std::cout << "connected to mongoDB ip : " <<  m_server.toAscii().data() << std::endl;


    std::cout << "connected to mongoDB base : " << m_database.toAscii().data() << std::endl;


    //m_server = server;
    //m_database = database;

    //this->gfs = new GridFS(this->mongo_connection, this->database.toAscii().data());
    //return this->gfs;

    try {
        this->m_mongo_connection.connect(this->m_server.toAscii().data());
        std::cout << "connected to mongoDB : " << this->m_server.toAscii().data() << std::endl;

        m_gfs = new GridFS(m_mongo_connection, m_database.toAscii().data());
        std::cout << "init GridFS" << std::endl;

    } catch(mongo::DBException &e ) {
        std::cout << "caught " << e.what() << std::endl;
        exit(1);
      }
}



Nosql::~Nosql()
{}


bo Nosql::Find(QString a_document, const bo &datas)
{
    qDebug() << "Nosql::Find";
    QString tmp;
    tmp.append(this->m_database).append(".").append(a_document);

    qDebug() << "m_database.a_document" << tmp;

    //std::cout << "element : " << element.jsonString(TenGen) << std::endl;

    //Query req = Query("{" + element.jsonString(TenGen) + "}" );
    //Query req = Query("{" + uuid.jsonString(TenGen) + "}, { _id : 1}" );

    try {
        auto_ptr<DBClientCursor> cursor = this->m_mongo_connection.query(tmp.toAscii().data(), mongo::Query(datas));

        qDebug() << "cursor created";
/*
        while( cursor->more() ) {
            result = cursor->next();
            //std::cout << "Nosql::Find pub uuid : " << datas.getField("os_version").valuestr() << std::endl;
            std::cout << "Nosql::Find _id : " << result.getField("_id").jsonString(Strict) << std::endl;
            // pub_uuid.append(host.getField("pub_uuid").valuestr());
        }*/

        if ( !cursor->more() )
                   return BSONObj();

        return cursor->nextSafe().copy();

    }
    catch(mongo::DBException &e ) {
        std::cout << "caught on find into " << m_server.toAscii().data() << "." << a_document.toAscii().data() << " : " << e.what() << std::endl;
        exit(1);
    }

}

/*
 *
 ExtractXML parse the deserialized job from Qpid server.
 Struct of the job from the Ruby code :
  job = {
    :email => @current_user.email,
    :uuid => params[:id], (Private host's uuid)
    :created_at => Time.now.utc,
    :_id => file_id (ID of the XML (GridFS id) sent from nodecastGUI)
  }
 *
 */
QDomDocument Nosql::ExtractXML(bo &a_datas)
{
    qDebug() << "Nosql::ExtractXML";
    //m_datas = a_datas;
    be gfs_id = a_datas.getField("_id");
    //be uuid = a_datas.getField("uuid");
    QDomDocument m_xml_datas;

    QString l_files;
    l_files.append(this->m_database).append(".").append("fs.files");
    QString l_chunks;
    l_chunks.append(this->m_database).append(".").append("fs.chunks");



    std::cout << "gfs_id : " << gfs_id.jsonString(TenGen) << std::endl;

    //Query req = Query("{" + uuid.jsonString(TenGen) + "}");


    if (ReadFile(gfs_id))
    {
        if (!this->m_gf->exists()) {
            std::cout << "file not found" << std::endl;
         }
        else {
            std::cout << "Find file !" << std::endl;

            QFile xml_tmp("/tmp/worker_nodecast_tmp.xml");

            m_gf->write(xml_tmp.fileName().toStdString().c_str());

            if( !m_xml_datas.setContent(&xml_tmp, false))
            {
                std::cout << "can not create XML" << std::endl;
                exit(1);
            }
            else
            {
                std::cout << "XML created !" << std::endl;
            }

            xml_tmp.close();
        }
    }


    std::cout << "before remove GridFS file" << std::endl;
    std::cout << "GridFS filename : " << this->m_gf->getFilename() << std::endl;
    std::cout << "GridFS file exist before : " << this->m_gf->exists() << std::endl;
    try {
        //this->m_gfs->removeFile(this->m_gf->getFilename());

        std::cout << "BSON : " << BSON( "files_id" << gfs_id) << std::endl;

        this->m_mongo_connection.remove( l_files.toAscii().data() , mongo::Query(gfs_id.wrap()) );
        this->m_mongo_connection.remove( l_chunks.toAscii().data() , BSON( "files_id" << gfs_id ) );

        std::cout << "after remove GridFS file" << std::endl;
    }
    catch(mongo::DBException &e)
    {
        std::cout << "caught on remove file : " << e.what() << std::endl;
        std::cout << "Nosql::ExtractXML GridFS file NOT FOUND : " << this->m_gf->getFilename() << std::endl;
    }

    std::cout << "GridFS file exist after : " << this->m_gf->exists() << std::endl;

    delete(this->m_gf);
    return m_xml_datas;
}


QBool Nosql::ReadFile(be &gfs_id)
{
    std::cout << "Nosql::ReadFile : " << gfs_id << std::endl;
    try {
        for (int i = 0; i < 5; i++)
        {
            this->m_gf = new mongo::GridFile(this->m_gfs->findFile(gfs_id.wrap()));
            qDebug() << "Nosql::ReadFile FOUND ";
            if (!this->m_gf->exists()) {
                std::cout << "file not found, sleep and retry, counter : " << i << std::endl;
                delete(this->m_gf);
                sleep(1);
            }
            else break;
        }
        return QBool(true);
    }
    catch(mongo::DBException &e ) {
        std::cout << "caught on get file : " << e.what() << std::endl;
        qDebug() << "Nosql::ReadFile NOT FOUND ";
        return QBool(false);
    }

}

QBool Nosql::Insert(QString a_document, bo a_datas)
{
    qDebug() << "Nosql::Insert";
    QString tmp;
    tmp.append(m_database).append(".").append(a_document);

    qDebug() << "Nosql::Insert tmp : " << tmp;


 try {
        this->m_mongo_connection.insert(tmp.toAscii().data(), a_datas);
        qDebug() << m_server + "." + a_document + " inserted";
        return QBool(true);
    }
    catch(mongo::DBException &e ) {
        std::cout << "caught on insert into " << m_server.toAscii().data() << "." << a_document.toAscii().data() << " : " << e.what() << std::endl;
        return QBool(false);
    }
}




QBool Nosql::Update(QString a_document, const bo &element_id, const bo &a_datas)
{
    qDebug() << "Nosql::Update";
    QString tmp;
    bo data = BSON( "$set" << a_datas);

    tmp.append(m_database).append(".").append(a_document);
    qDebug() << "Nosql::Update tmp : " << tmp;

 try {
        this->m_mongo_connection.update(tmp.toAscii().data(), mongo::Query(element_id), data);
        qDebug() << m_server + "." + a_document + " updated";
        return QBool(true);
    }
    catch(mongo::DBException &e ) {
        std::cout << "caught on update into " << m_server.toAscii().data() << "." << a_document.toAscii().data() << " : " << e.what() << std::endl;
        return QBool(false);
    }
}
