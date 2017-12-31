#ifndef NODESTORE_H
#define NODESTORE_H
#include <QAbstractListModel>
#include "node.h"
#include "sensorstore.h"

class node;
class nodestore : public QAbstractListModel
{
public:
    nodestore();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &/*parent*/) const { return 1; }

    QHash<int, QByteArray> roleNames() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    void updateWith(QHostAddress nodeip, QVariantMap nodeinfo);

private:
    QVector<node*> nodes;
    sensorstore* allsensorslist;
};

#endif // NODESTORE_H
