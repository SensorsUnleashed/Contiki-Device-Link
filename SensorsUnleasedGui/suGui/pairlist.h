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
