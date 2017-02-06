#ifndef PAIRLIST_H
#define PAIRLIST_H

#include "node.h"
#include "sensorstore.h"
#include <QAbstractListModel>
#include <QMultiHash>
#include <QObject>

class sensor;
class sensorstore;
class pairlist : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit pairlist(sensor* pairowner, sensorstore* s);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }

    QHash<int, QByteArray> roleNames() const;

    void addSensor(sensor* s);
    void append(QVariantMap dstinfo);

private:
    QMultiHash<sensor*, sensor*> pairs;
    sensorstore* allsensorslist;
    sensor* owner;
};

#endif // PAIRLIST_H
