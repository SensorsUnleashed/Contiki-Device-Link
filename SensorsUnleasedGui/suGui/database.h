/*******************************************************************************
 * Copyright (c) 2017, Ole Nissen.
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  1. Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution. 
 *  3. The name of the author may not be used to endorse or promote
 *  products derived from this software without specific prior
 *  written permission.  
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Sensors Unleashed project
 *******************************************************************************/
#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QObject>
#include <QVariant>
#include <QDebug>

#define DBFILEPATH  "Database/setup.db"
class database : public QObject
{
    Q_OBJECT
public:
    explicit database(){
        qDebug() << "databasefile: " << DBFILEPATH;
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(DBFILEPATH);

        if(db.open()){
            qDebug() << "Database opened successfully";
        }
        else{
            qDebug() << "Database could not be opened";
        }
    }

    /*
     * Returns:
     *  -1 for error
     *   0 for success
    */
    int query(QString querystring, QVariantList* result){

        if(db.open()){
            QSqlQuery query(querystring);
            QSqlRecord rec = query.record();
            while(query.next()){
                QVariantMap map;
                for(int i=0; i<rec.count(); i++){
                    map.insert(rec.fieldName(i), query.value(i));
                }
                result->append(map);
            }
        }
        else{
            return -1;
        }

        return 0;
    }

    int update(QString table, QVariantList data){
        if(db.open()){
            QSqlQuery query;
            for(int i=0; i<data.count(); i++){
                QVariantMap item = data.at(i).toMap();
                QString sqlupdatestring = \
                        "UPDATE " + table + " SET Value=" + QString::number(item["Value"].toInt()) +
                        " WHERE Key='" + item["Key"].toString() + "';";
                if(!query.exec(sqlupdatestring)){
                    qDebug() << query.lastError();
                }
            }
        }
        else{
            return -1;
        }
        return 0;
    }

    int insert(QString querystring){
        if(db.open()){
            QSqlQuery query;
            if(!query.exec(querystring)){
                qDebug() << query.lastError();
                return 1;
            }
            return 0;
        }
    }

private:
    QSqlDatabase db;
};
#endif // DATABASE_H

