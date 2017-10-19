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
#ifndef NODE_H
#define NODE_H

#include <QObject>

#include <QHostAddress>
#include "../../sensorsUnleashed/lib/cmp_helpers.h"
#include "wsn.h"
#include "pairlist.h"
#include "sensorstore.h"

enum request{
    req_RangeMinValue,
    req_RangeMaxValue,

    req_currentValue,
    req_observe,
    req_aboveEventValue,
    req_belowEventValue,
    req_changeEventAt,

    req_getEventSetup,
    req_updateEventsetup,

    req_pairingslist,
    req_clearparings,
    req_removepairingitems,
    req_pairsensor,
    /* Used to set a command for an actuator
     * could be togglerelay or other
    */
    req_setCommand,

    /* Used to test a device event handler*/
    req_testevent,

    /* Internal gui events */
    observe_monitor,

    /* System control */
    format_filesystem,
};

struct msgid_s{
    uint16_t number;
    enum request req;
};
typedef struct msgid_s msgid;

class pairlist;
class sensorstore;
class node;

class suinterface : public wsn
{
    Q_OBJECT
public:
    suinterface(QHostAddress addr);

protected:
    QVector<msgid> token;

    uint16_t get_request(CoapPDU *pdu, enum request req, QByteArray payload=0);
    uint16_t put_request(CoapPDU *pdu, enum request req, QByteArray payload=0);

private:

};

class sensor : public suinterface
{
    Q_OBJECT
public:
    sensor(node* parent, QString uri, QVariantMap attributes, sensorstore *p);

    //Dummy constructor
    sensor(QString ipaddr, QString uri);

    node* getParent(){ return parent;}
    QString getUri(){ return uri; }
    QString getAddressStr() {return ip.toString(); }

    void initSensor();
    Q_INVOKABLE void requestRangeMin();
    Q_INVOKABLE void requestRangeMax();

    Q_INVOKABLE void requestValue();
    Q_INVOKABLE QVariant requestObserve();
    Q_INVOKABLE void abortObserve(QVariant token);
    Q_INVOKABLE void requestAboveEventLvl();
    Q_INVOKABLE void requestBelowEventLvl();
    Q_INVOKABLE void requestChangeEventLvl();

    Q_INVOKABLE void req_eventSetup();
    Q_INVOKABLE void updateConfig(QVariant updatevalues);
    Q_INVOKABLE QVariant getConfigValues();   //Get last stored values without quering the sensor

    /* Pair this sensor with another. */
    Q_INVOKABLE void getpairingslist();
    Q_INVOKABLE QVariant clearpairingslist();
    Q_INVOKABLE uint16_t removeItems(QByteArray arr);
    Q_INVOKABLE QVariant pair(QVariant pairdata);
    int parsePairList(cmp_ctx_t* cmp);
    pairlist* getPairListModel() { return pairings; }

    int addDummyPair(QString ip, QString dsturi, QString url);

    Q_INVOKABLE void testEvents(QVariant event, QVariant value);

    void handleReturnCode(uint16_t token, CoapPDU::Code code);
    void nodeNotResponding(uint16_t token);
    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload, CoapPDU::Code code);

    virtual QVariant getClassType(){ return "DefaultSensor.qml"; }
    Q_INVOKABLE virtual QVariant getActionModel() { return "DefaultActions.qml"; }

protected:
    QString uri;

    //uint16_t put_request(CoapPDU *pdu, enum request req, QByteArray payload);
private:
    node* parent;
    QVariantMap sensorinfo;
//    QVector<msgid> token;
    pairlist* pairings;
    QHostAddress ip;
    uint8_t init;   //Flag to indicate if sensor config has been requested or not

    cmp_object_t eventsActive;		//All events on or Off
    cmp_object_t LastValue;
    cmp_object_t AboveEventAt;	//When resource crosses this line from low to high give an event (>=)
    cmp_object_t BelowEventAt;	//When resource crosses this line from high to low give an event (<=)
    cmp_object_t ChangeEvent;	//When value has changed more than changeEvent + lastevent value <>= value
    cmp_object_t RangeMin;		//What is the minimum value this device can read
    cmp_object_t RangeMax;		//What is the maximum value this device can read

    //uint16_t get_request(CoapPDU *pdu, enum request req, QByteArray payload=0);

signals:
    void currentValueChanged(QVariant result);
    void observe_started(QVariant result, uint16_t token);
    void observe_failed(uint16_t token);
    void aboveEventValueChanged(QVariant result);
    void belowEventValueChanged(QVariant result);
    void changeEventValueChanged(QVariant result);

    void eventSetupRdy();

    void rangeMaxValueReceived(QVariant result);
    void rangeMinValueReceived(QVariant result);
};

class pulsecounter : public sensor {
    Q_OBJECT
public:
    pulsecounter(node* parent, QString uri, QVariantMap attributes, sensorstore *p);
    QVariant getClassType(){ return "PulseCounter.qml"; }

    Q_INVOKABLE void startPoll(QVariant interval);

private:
    QTimer* polltimer;

private slots:
    void doPoll();

};

class defaultdevice : public sensor {
    Q_OBJECT
public:
    defaultdevice(node *parent, QString uri, QVariantMap attributes, sensorstore *p);

    QVariant getClassType(){ return "DefaultDevice.qml"; }

    Q_INVOKABLE void setToggle();
    Q_INVOKABLE void setOn();
    Q_INVOKABLE void setOff();
};

class nodeinfo : public suinterface
{
    Q_OBJECT
public:
    nodeinfo(node *parent, QString uri, QVariantMap attributes);

    uint16_t request_cfs_format();

    void handleReturnCode(uint16_t token, CoapPDU::Code code);

private:
    node* parent;
    QString uri;
};

class node : public wsn
{
    Q_OBJECT
public:
    node(QHostAddress addr, QVariantMap data, sensorstore* p);
    void addSensor(QString uri, QVariantMap attributes);

    QVariant getDatabaseinfo(){ return databaseinfo; }
    QHostAddress getAddress() { return ip; }
    QString getAddressStr() {return ip.toString(); }
    sensor* getSensor(QString addr);

    Q_INVOKABLE nodeinfo* getNodesetupdevice(){ return nodesetup; }
    Q_INVOKABLE void getSensorslist();
    Q_INVOKABLE void requestLinks();

    Q_INVOKABLE void request_cfs_format(){ if(nodesetup) nodesetup->request_cfs_format(); }

    QVector<sensor*> getSensorslistRaw(){ return sensors; }


    /* Virtual functions (wsn)*/
    void nodeNotResponding(uint16_t token);
    QVariant parseAppLinkFormat(uint16_t token, QByteArray payload);
    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload);

private:
    QString name;
    QHostAddress ip;
    QVariantMap linklist;
    uint16_t token;
    sensorstore* allsensorslist;
    sensorstore* ownsensorslist;

    QVariantMap databaseinfo;

    //TBD
    QVector<sensor*> sensors;

    nodeinfo* nodesetup;

signals:
    void sensorFound(QVariant sensorinfo, QVariant source);

public slots:
};


#endif // NODE_H
