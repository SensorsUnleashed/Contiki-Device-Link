#include "pairlist.h"
#include "node.h"

pairlist::pairlist(sensor* pairowner, sensorstore *s) : QAbstractListModel()
{
    owner = pairowner;
    allsensorslist = s;
}

enum roles {
    sensorname = Qt::UserRole,
    nodename,
    nodeip,
};

QHash<int, QByteArray> pairlist::roleNames() const{
    QHash<int, QByteArray> rolelist;
    rolelist.insert(sensorname, QByteArray("sensorname"));
    rolelist.insert(nodename, QByteArray("nodename"));
    rolelist.insert(nodeip, QByteArray("nodeip"));
    return rolelist;
}

QVariant pairlist::data(const QModelIndex& index, int role) const{
    if (!index.isValid())
        return QVariant();


    QHash<sensor*, sensor*>::const_iterator i = pairs.cbegin() + index.row();
    if(i == pairs.cend()) {
        return QVariant();
    }

    int dummysensor = i.value()->getParent() == 0;

    if(role == sensorname){
        return i.value()->getUri();
    }
    else if(role == nodename){
        if(dummysensor){
            return QVariant("Dummy");
        }
        return i.value()->getParent()->getDatabaseinfo().toMap()["name"];
    }
    else if(role == nodeip){
        return i.value()->getAddressStr();
    }

    return QVariant();
}

int pairlist::rowCount(const QModelIndex&) const{
    return pairs.count();
}

void pairlist::append(QVariantMap dstinfo){

    /* Check if destination sensor is known to the system */
    sensor* dst = allsensorslist->find(dstinfo["addr"].toString(), dstinfo["dsturi"].toString());

    if(dst == 0){
        //We need to create a dummy sensor, so that we have something to display. Will not work
        dst = new sensor(dstinfo["addr"].toString(), dstinfo["dsturi"].toString());
        allsensorslist->append(dst);
    }

    if(!pairs.contains(owner, dst)){
        beginInsertRows(QModelIndex(), rowCount(), rowCount()); //Append to the end of the list
        pairs.insert(owner, dst);
        endInsertRows();
    }
}

