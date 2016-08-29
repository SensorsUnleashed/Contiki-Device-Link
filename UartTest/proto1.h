#ifndef PROTO1_H
#define PROTO1_H

#include "uart.h"
#include <QObject>

class proto1: public QObject
{
    Q_OBJECT

public:
    proto1(uart *comm);

private:

    QVector<QString> resources;

    enum commands {
        resource_count = 1,  //Get the total no of resources
        resource_url,      //Get a specific resource
        debugstring,
    };

    uart* comm;
    
    int parseMessage(QByteArray& msg);
    
private slots:
    void handleIncomming();
};

#endif // PROTO1_H
