#include "sensorsunleashed.h"
#include "../../apps/uartsensors/uart_protocolhandler.h"

#include "helper.h"

sensorsunleashed::sensorsunleashed(database *db, coaphandler *coap, QQmlContext *context)
{
    this->db = db;
    this->context = context;
    nodecomm = coap;
    allsensorslist = new sensorstore();

    QVariantList result;
    QString querystring = "select * from nodes;";
    //Read settings from the database
    if(db->query(querystring, &result) == 0){
        for(int i=0; i<result.count(); i++){
            QVariantMap n = result.at(i).toMap();
            createNode(n);
        }
    }

    result.clear();
    querystring = "select * from sensors;";
    //Read settings from the database
    if(db->query(querystring, &result) == 0){
        for(int i=0; i<result.count(); i++){
            QVariantMap s = result.at(i).toMap();
            node* n = findNode(s["parentnodeaddr"].toString());
            if(n != 0){
                n->addSensor(s["uri"].toString(), s["attributes"].toMap());
            }
        }
    }

    /* Start listening to the */
    connect(allsensorslist, SIGNAL(sensorAdded(sensor*)), this, SLOT(updateDB(sensor*)));
}

void sensorsunleashed::updateDB(sensor* s){
    QString querystring = "INSERT INTO sensors VALUES (\"" + s->getAddressStr() + "\", \"" + s->getUri() + "\", NULL);";
    if(db->insert(querystring) == 0){
        qDebug() << "Sensor: " << s->getUri() << " from node: " << s->getParent()->getDatabaseinfo().toMap()["name"] << " added to database";
    }
}

node* sensorsunleashed::findNode(QString nodeid){
    for(int i=0; i<nodes.count(); i++){
        if(nodes.at(i)->getAddressStr().compare(nodeid) == 0)
            return nodes.at(i);
    }
    return 0;
}

void sensorsunleashed::changeActiveNode(QVariant nodeinfo){
    QVariantMap ninfo = nodeinfo.toMap();
    node* n = findNode(ninfo["address"].toString());

    if(n ==0) return;
    context->setContextProperty("activeNode", n);
    qDebug() << "Active node changed to: " << n->getAddressStr();
}

QVariant sensorsunleashed::changeActiveSensor(QVariant sensorinfo){
    QVariantMap sinfo = sensorinfo.toMap();
    node* n = findNode(sinfo["node"].toString());
    if(n == 0) QVariant(1);

    sensor* s = n->getSensor(sinfo["sensor"].toString());
    if(s == 0) QVariant(1);

    context->setContextProperty("activeSensor", s);
    context->setContextProperty("pairlist", s->getPairListModel());
    s->initSensor();
    qDebug() << "Active sensor changed to: " << s->getUri();

    return QVariant(0);
}



/* Used to initialize the list of nodes in the gui */
void sensorsunleashed::initNodelist(){
    for(int i=0; i<nodes.count(); i++){
        QVariant nodeinfo = nodes.at(i)->getDatabaseinfo();
        emit nodeCreated(nodeinfo);
    }
}

/* Returns:
 * 0 = Success - node created
 * 1 = ip could not be converted to QHostAddrss format
 * 2 = Node is already known. Not created
*/
QVariant sensorsunleashed::createNode(QVariant nodeinfo){

    QVariantMap map = nodeinfo.toMap();
    QHostAddress a(map["address"].toString());
    if(a.isNull()){
        return QVariant(1);
    }

    for(int i=0; i<nodes.count(); i++){
        if(nodes.at(i)->getAddress() == a){
            return QVariant(2);
        }
    }

    node* n = new node(a, nodeinfo.toMap(), allsensorslist);
    nodes.append(n);
    emit nodeCreated(nodeinfo);
    return QVariant(0);
}

QVariantList sensorsunleashed::getAllSensorsList(){
    QVariantList list;
    for(int i=0; i<nodes.count(); i++){
        QVector<sensor*> slist = nodes.at(i)->getSensorslistRaw();

        for(int j=0; j<slist.count(); j++){
            QVariantMap item;
            item["node_name"] = nodes.at(i)->getDatabaseinfo().toMap()["name"];
            item["node_addr"] = nodes.at(i)->getAddressStr();
            item["sensor_name"] = slist.at(j)->getUri();
            list.append(item);
        }
    }
    return list;
}

