#ifndef NODE_H
#define NODE_H

#include <QObject>

#include <QHostAddress>
#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "wsn.h"
#include "pairlist.h"
#include "sensorstore.h"

enum request{
    req_RangeMinValue,
    req_RangeMaxValue,

    req_currentValue,
    req_aboveEventValue,
    req_belowEventValue,
    req_changeEventAt,

    req_getEventSetup,
    req_updateEventsetup,

    req_pairingslist,
    req_pairsensor,
    /* Used to set a command for an actuator
     * could be togglerelay or other
    */
    req_setCommand,

    /* Used to test a device event handler*/
    req_testevent,
};

struct msgid_s{
    uint16_t number;
    enum request req;
};
typedef struct msgid_s msgid;

class pairlist;
class sensorstore;
class node;
class sensor : public wsn
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
    Q_INVOKABLE void requestAboveEventLvl();
    Q_INVOKABLE void requestBelowEventLvl();
    Q_INVOKABLE void requestChangeEventLvl();

    Q_INVOKABLE void req_eventSetup();
    Q_INVOKABLE void updateConfig(QVariant updatevalues);
    Q_INVOKABLE QVariant getConfigValues();   //Get last stored values without quering the sensor

    /* Pair this sensor with another. */
    Q_INVOKABLE void getpairingslist();
    Q_INVOKABLE QVariant pair(QVariant pairdata);
    int parsePairList(cmp_ctx_t* cmp);
    pairlist* getPairListModel() { return pairings; }

    Q_INVOKABLE void testEvents(QVariant event, QVariant value);

    void nodeNotResponding(uint16_t token);
    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload);

    virtual QVariant getClassType(){ return "SensorInformation.qml"; }
protected:
    QString uri;

    void put_request(CoapPDU *pdu, enum request req, QByteArray payload);
private:
    node* parent;
    QVariantMap sensorinfo;
    QVector<msgid> token;
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

    void get_request(CoapPDU *pdu, enum request req, QByteArray payload=0);

signals:
    void currentValueChanged(QVariant result);
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

class powerrelay : public sensor {
    Q_OBJECT
public:
    powerrelay(node* parent, QString uri, QVariantMap attributes, sensorstore *p);
    QVariant getClassType(){ return "PowerRelay.qml"; }

    Q_INVOKABLE void toggleRelay();
    Q_INVOKABLE void setOn();
    Q_INVOKABLE void setOff();

private:
};

class ledindicator : public sensor {
    Q_OBJECT
public:
    ledindicator(node *parent, QString uri, QVariantMap attributes, sensorstore *p);
    QVariant getClassType(){ return "LedIndicator.qml"; }

    Q_INVOKABLE void toggleRedLED();
    Q_INVOKABLE void toggleGreenLED();
    Q_INVOKABLE void toggleOrangeLED();
    Q_INVOKABLE void toggleYellowLED();
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

    Q_INVOKABLE void getSensorslist();
    Q_INVOKABLE void requestLinks();

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

signals:
    void sensorFound(QVariant sensorinfo, QVariant source);

public slots:
};


#endif // NODE_H
