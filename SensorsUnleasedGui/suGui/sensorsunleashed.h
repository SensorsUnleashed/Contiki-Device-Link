#ifndef SENSORSUNLEASHED_H
#define SENSORSUNLEASHED_H

#include "database.h"
#include "coaphandler.h"
#include "node.h"
#include <QObject>

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



    Q_INVOKABLE QVariant pair(QVariant nodeaddr, QVariant uri, QVariant options, QVariant pairdata, QVariant oldtoken);
    Q_INVOKABLE QVariant put(QVariant nodeaddr, QVariant uri, QVariant options, QVariant data, QVariant oldtoken);

private:
    database *db;
    coaphandler* nodecomm;
    QVector<node*> nodes;

    QQmlContext *context;
signals:

    void nodeCreated(QVariant nodeinfo);

public slots:

};

#endif // SENSORSUNLEASHED_H
