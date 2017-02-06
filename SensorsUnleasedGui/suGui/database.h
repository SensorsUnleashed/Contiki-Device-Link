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

