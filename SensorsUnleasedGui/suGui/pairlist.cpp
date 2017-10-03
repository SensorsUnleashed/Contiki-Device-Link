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
    selected,
    eventSetupQML,
};

QHash<int, QByteArray> pairlist::roleNames() const{
    QHash<int, QByteArray> rolelist;
    rolelist.insert(sensorname, QByteArray("sensorname"));
    rolelist.insert(nodename, QByteArray("nodename"));
    rolelist.insert(nodeip, QByteArray("nodeip"));
    rolelist.insert(selected, QByteArray("selected"));
    rolelist.insert(eventSetupQML, QByteArray("eventSetupQML"));
    return rolelist;
}

QVariant pairlist::data(const QModelIndex& index, int role) const{
    if (!index.isValid())
        return QVariant();

    int dummysensor = pairs.at(index.row())->dst->getParent() == 0;

    if(role == sensorname){
        return pairs.at(index.row())->dst->getUri();
    }
    else if(role == nodename){
        if(dummysensor){
            return QVariant("Dummy");
        }
        return pairs.at(index.row())->dst->getParent()->getDatabaseinfo().toMap()["name"];
    }
    else if(role == nodeip){
        return pairs.at(index.row())->dst->getAddressStr();
    }
    else if(role == selected){
        return pairs.at(index.row())->selected;
    }
    else if(role == eventSetupQML){
        return pairs.at(index.row())->eventSetupQMLlnk;
    }

    return QVariant();
}



int pairlist::rowCount(const QModelIndex&) const{
    return pairs.count();
}

Qt::ItemFlags pairlist::flags(const QModelIndex &index) const
{
    //    if (index.isValid())
    //        return (QAbstractListModel::flags(index)|Qt::ItemIsSelect able);

    return Qt::ItemIsSelectable;
}

void pairlist::setSelected(int row, const QVariant &value)
{
    setData(index(row, 0), value, selected);
}

bool pairlist::setData(const QModelIndex &index, const QVariant &value, int role){

    if(!index.isValid()) return false;

    if(role == selected){
        pairs.at(index.row())->selected = value.toUInt();
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool pairlist::removeRows(int row, int count, const QModelIndex &parent){
    beginRemoveRows(QModelIndex(), row, row+count); //Append to beginning end of the list
    pairs.remove(row, count);
    endRemoveRows();
}

void pairlist::clear(){
    beginRemoveRows(QModelIndex(), 0, rowCount()); //Append to beginning end of the list
    pairs.clear();
    endRemoveRows();
}

void pairlist::append(QVariantMap dstinfo){

    /* Check if destination sensor is known to the system */
    sensor* dst = allsensorslist->find(dstinfo["addr"].toString(), dstinfo["dsturi"].toString());

    if(dst == 0){
        //We need to create a dummy sensor, so that we have something to display. Will not work
        dst = new sensor(dstinfo["addr"].toString(), dstinfo["dsturi"].toString());
        allsensorslist->append(dst);
    }

    struct aPair* p = new struct aPair;
    p->dst = dst;
    p->id = rowCount(); //Workaround for now!! dstinfo["id"].toUInt();
    p->selected = 0;
    p->eventSetupQMLlnk = "PowerRelayEventSetup.qml";

    if(!pairs.contains(p)){
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        pairs.append(p);
        endInsertRows();
    }
}

void pairlist::removePairings(){
    QByteArray a;
    for(int i=0; i<rowCount(); i++){
        if(pairs.at(i)->selected == 1){
            a.append(pairs.at(i)->id);
        }
    }

    if(a.size() == rowCount()){ //Just wipe out the remote pairingsfile
        lastcmd.number = owner->clearpairingslist();
    }
    else{
        lastcmd.number = owner->removeItems(a);
    }
    lastcmd.rowstodelete = a;
}

void pairlist::removePairingsAck(){
    for(int i=0; i<lastcmd.rowstodelete.size(); i++){
        for(int j=0; j<rowCount(); j++){
            if(pairs.at(j)->id == lastcmd.rowstodelete.at(i)){
                removeRow(j);
                break;
            }
        }
    }
}
