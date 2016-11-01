#ifndef SENSORSUNLEASHED_H
#define SENSORSUNLEASHED_H

#include "database.h"
#include "coaphandler.h"
#include <QObject>

class sensorsunleashed : public QObject
{
    Q_OBJECT
public:
    explicit sensorsunleashed(database* db, coaphandler* coap);

    Q_INVOKABLE QVariant pair(QVariant nodeaddr, QVariant uri, QVariant options, QVariant pairdata, QVariant oldtoken);

private:
    coaphandler* nodecomm;
signals:

public slots:
};

#endif // SENSORSUNLEASHED_H
