#ifndef GUIGLUE_H
#define GUIGLUE_H

#include "proto1.h"
#include <QObject>

struct deviceinfo_s {
    cmp_object_t lastval;
    struct resourceconf conf;
};


class guiglue : public QObject
{
    Q_OBJECT
public:
    explicit guiglue(proto1* protohandler);

private:
    proto1 *interface;
    char* stringbuffer;
    QVector<struct deviceinfo_s> devinfo;

signals:

public slots:
    void updateValue(QVariant id);
    void updateTimerValue();

private slots:
    void reqResourceCount(rx_msg* rx_req);
    void reqConfig(rx_msg *rx_req);
    void reqValueUpdateAll(rx_msg* rx_req);
    void reqValueUpdate(rx_msg *rx_req);
};

#endif // GUIGLUE_H
