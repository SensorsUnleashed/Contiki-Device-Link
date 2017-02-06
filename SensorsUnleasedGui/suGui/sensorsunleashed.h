#ifndef SENSORSUNLEASHED_H
#define SENSORSUNLEASHED_H

#include "database.h"
#include "coaphandler.h"
#include "node.h"
#include <QObject>
#include "sensorstore.h"

#include <QQmlContext>


/* This class serves as an interface between the the GUI and the c++, at
 * least until
*/
class sensorsunleashed : public QObject
{
    Q_OBJECT
public:
    explicit sensorsunleashed(database* db, coaphandler* coap, QQmlContext *context);

    Q_INVOKABLE void changeActiveNode(QVariant nodeinfo);
    Q_INVOKABLE QVariant changeActiveSensor(QVariant sensorinfo);

    Q_INVOKABLE void initNodelist();

    node* findNode(QString nodeid);
    Q_INVOKABLE QVariant createNode(QVariant nodeinfo);

    Q_INVOKABLE QVariantList getAllSensorsList();

private:
    database *db;
    coaphandler* nodecomm;
    QVector<node*> nodes;
    sensorstore* allsensorslist;
    QQmlContext *context;
signals:


    void nodeCreated(QVariant nodeinfo);

private slots:
    void updateDB(sensor* s);

public slots:


};

#endif // SENSORSUNLEASHED_H
