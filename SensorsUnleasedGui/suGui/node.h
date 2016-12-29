#ifndef NODE_H
#define NODE_H

#include <QObject>

#include <QHostAddress>
#include "../../apps/uartsensors/uart_protocolhandler.h"
#include "wsn.h"

enum request{
    req_RangeMinValue,
    req_RangeMaxValue,

    req_currentValue,
    req_aboveEventValue,
    req_belowEventValue,
    req_changeEventAt,

    req_getEventSetup,
    req_updateEventsetup,
};

struct msgid_s{
    uint16_t number;
    enum request req;
};
typedef struct msgid_s msgid;

class node;
class sensor : public wsn
{
    Q_OBJECT
public:
    sensor(node* parent, QString uri, QVariantMap attributes);

    QString getUri(){ return uri; }

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

    void nodeNotResponding(uint16_t token);
    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload);

    /* Pair this sensor with another. */
    QVariant pair(QVariant pairdata);

protected:
private:
    QString uri;
    QVariantMap sensorinfo;
    QVector<msgid> token;
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
    void put_request(CoapPDU *pdu, enum request req, QByteArray payload);
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
public:
    pulsecounter();

private:
};


class node : public wsn
{
    Q_OBJECT
public:
    node(QHostAddress addr, QVariantMap data);

    QVariant getDatabaseinfo(){ return databaseinfo; }
    QHostAddress getAddress() { return ip; }
    QString getAddressStr() {return ip.toString(); }
    sensor* getSensor(QString addr);

    Q_INVOKABLE void getSensorslist();
    Q_INVOKABLE void requestLinks();

    /* Virtual functions (wsn)*/
    void nodeNotResponding(uint16_t token);
    QVariant parseAppLinkFormat(uint16_t token, QByteArray payload);
    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload);

private:
    QString name;
    QHostAddress ip;
    QVariantMap linklist;
    uint16_t token;

    QVariantMap databaseinfo;

    QVector<sensor*> sensors;

signals:
    void sensorFound(QVariant sensorinfo);

public slots:
};


#endif // NODE_H
