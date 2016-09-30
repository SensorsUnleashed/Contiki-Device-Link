#ifndef PROTO1_H
#define PROTO1_H

#include "uart.h"
#include <QObject>

extern "C" {
#include "../coap-uart-device-handler/coap_proxy_protocolhandler.h"
}

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



class proto1: public uart
{
    Q_OBJECT

public:
    proto1();

    int tx_value(cmp_object_t* payload);

    int frameandtx(uint8_t *payload, uint8_t len, uint8_t seqno = 255);
    int frameandtx(uint8_t id, cmp_object_t* value, uint8_t seqno = 255);

private:
    uart* comm;
    QByteArray currentmsg;
    int parseMessage(QByteArray msg);
    
private slots:
    void handleIncomming(buffer_t *rxbuf);

signals:
    void reqResourceCount(rx_msg* rx_req);
    void reqConfig(rx_msg* rx_req);
    void reqValueUpdate(rx_msg* rx_req);
};

#endif // PROTO1_H
