#ifndef SENSORSTORE_H
#define SENSORSTORE_H

#include <QObject>
#include <QAbstractListModel>
#include "node.h"

class sensor;
class sensorstore : public QAbstractListModel
{
    Q_OBJECT
public:
    sensorstore();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }

    sensor* find(QString ipaddr, QString uri);
    void append(sensor* s);

signals:
    void sensorAdded(sensor* s);
private:
    QVector<sensor*> list;
};

#endif // SENSORSTORE_H
