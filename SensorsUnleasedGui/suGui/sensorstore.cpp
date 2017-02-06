#include "sensorstore.h"

sensorstore::sensorstore()
{

}

QVariant sensorstore::data(const QModelIndex &index, int role) const{

}

int sensorstore::rowCount(const QModelIndex &parent) const{

}

sensor* sensorstore::find(QString ipaddr, QString uri){
    /* Check if we have the dst sensor in our list (We might not) */
    for(int i=0; i<list.count(); i++){
        if(list.at(i)->getAddressStr().compare(ipaddr) == 0){
            if(list.at(i)->getUri().compare(uri) == 0){
                return list.at(i);
            }
        }
    }
    return 0;
}

void sensorstore::append(sensor* s){
    if(!list.contains(s)){
        list.append(s);
        emit sensorAdded(s);
    }
}

