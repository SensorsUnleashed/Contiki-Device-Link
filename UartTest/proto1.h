#ifndef PROTO1_H
#define PROTO1_H

#include "uart.h"
#include <QObject>

typedef enum {
  NO_FLAGS = 0,

  /* methods to handle */
  METHOD_GET = (1 << 0),
  METHOD_POST = (1 << 1),
  METHOD_PUT = (1 << 2),
  METHOD_DELETE = (1 << 3),

  /* special flags */
  HAS_SUB_RESOURCES = (1 << 4),
  IS_SEPARATE = (1 << 5),
  IS_OBSERVABLE = (1 << 6),
  IS_PERIODIC = (1 << 7)
} rest_resource_flags_t;

class proto1: public QObject
{
    Q_OBJECT

public:
    proto1(uart *comm);

private:

    QVector<QVariantMap> resources;

    enum commands {
        resource_count = 1,     //Get the total no of resources
        resource_url,           //Get a specific resource uri
        resource_attributes,    //Get a specific resource attribute
        resource_flags,
        debugstring,
    };



    uart* comm;
    
    int parseMessage(QByteArray& msg);
    
private slots:
    void handleIncomming();
};

#endif // PROTO1_H
