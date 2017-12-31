#include "nodestore.h"

enum NodeRoles {
    IPAdress = Qt::UserRole + 1,
    Name,
    Location,
    LastSeen,
    NodeInfo,
    BorderColor
};


nodestore::nodestore()
{
    allsensorslist = new sensorstore();
}

QHash<int, QByteArray> nodestore::roleNames() const{
    QHash<int, QByteArray> roles;
    roles[IPAdress] = "IPAdress";
    roles[Name] = "Name";
    roles[Location] = "Location";
    roles[LastSeen] = "LastSeen";
    roles[NodeInfo] = "NodeInfo";
    roles[BorderColor] = "BorderColor";
    return roles;
}


QVariant nodestore::data(const QModelIndex &index, int role) const{
    if(!index.isValid()) return QVariant(0);

    node* n = nodes[index.row()];
    if(role == NodeRoles::IPAdress){
        return n->getAddress().toString();
    }
    else if(role == NodeRoles::Name){
        QVariantMap info = n->getDatabaseinfo().toMap();
        if(info.contains("name")){
            return info["name"].toString();
        }
        else{
            return "";
        }
    }
    else if(role == NodeRoles::Location){
        QVariantMap info = n->getDatabaseinfo().toMap();
        if(info.contains("location")){
            return info["location"].toString();
        }
        else{
            return "";
        }
    }
    else if(role == NodeRoles::LastSeen){
        return n->getLastSeenTime();
    }
    else if(role == NodeRoles::NodeInfo){
        return n->getDatabaseinfo();
    }
    else if(role == BorderColor){

        QDateTime t = n->getLastSeenTime();
        if(!t.isValid()){   //Never seen
            return "black";
        }
        int sec = t.secsTo(QDateTime::currentDateTime());

        if(sec > 3600) { //If its more than 1 hour since 1*60*60
            return "red";
        }
        else if(sec > 1800 && sec < 3600) { //If its less than 1 hour but more than ½ hour since
            return "yellow";
        }
        else{
            return "green";
        }
    }

    return QVariant(0);
}


int nodestore::rowCount(const QModelIndex &parent) const{
    (void)parent;
    return nodes.count();
}


Qt::ItemFlags nodestore::flags(const QModelIndex &index) const {
    (void)index;
    return Qt::ItemIsEditable;
}

bool nodestore::setData(const QModelIndex &index, const QVariant &value, int role){
    if(!index.isValid()) return false;

    node* n = nodes[index.row()];
    if(role == NodeRoles::LastSeen){
        n->updateLastSeenTime(value.toInt());
        dataChanged(index, index);
        return true;
    }

    return false;
}

void nodestore::updateWith(QHostAddress nodeip, QVariantMap nodeinfo){

    for(int i=0; i<rowCount(); i++){
        //QModelIndex index(i, 0);
        QModelIndex in = index(i);
        QHostAddress addr(data(in, NodeRoles::IPAdress).toString());

        //data(index, IPAdress)
        if(nodeip == addr){
            //Update with time, if availble
            if(nodeinfo.contains("LastSeen")){
                setData(in, nodeinfo["LastSeen"], NodeRoles::LastSeen);
                return;
            }
        }
    }

    /* Node not found - lets create a new */
    //QModelIndex in(row);
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    node* n = new node(nodeip, nodeinfo, allsensorslist);
    nodes.append(n);
    endInsertRows();
}
