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
#ifndef PAIRLIST_H
#define PAIRLIST_H

#include "node.h"
#include "sensorstore.h"
#include <QAbstractListModel>
#include <QMultiHash>
#include <QObject>

/*
 * A pair is made from:
 *
*/

class sensor;
class sensorstore;

struct aPair{
    uint8_t id;
    sensor* dst;
    uint8_t selected;
    QVariant eventSetupQMLlnk;
};



class pairlist : public QAbstractListModel
{
    Q_OBJECT
public:

    struct ackCmd{
        uint16_t number;
        QByteArray rowstodelete;
    };

    explicit pairlist(sensor* pairowner, sensorstore* s);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }
    Qt::ItemFlags flags(const QModelIndex &index) const;

    Q_INVOKABLE void setSelected(int row, const QVariant &data);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    QHash<int, QByteArray> roleNames() const;

    void addSensor(sensor* s);
    void clear();
    Q_INVOKABLE void removePairings();
    void removePairingsAck();

    void append(QVariantMap dstinfo);

private:
    struct ackCmd lastcmd;
    QVector<struct aPair*> pairs;
    sensorstore* allsensorslist;
    sensor* owner;
};

#endif // PAIRLIST_H
