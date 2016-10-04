#include "proto1.h"
#include <QDebug>

extern "C" {
#include "../coap-uart-device-handler/coap_proxy_protocolhandler.h"

#include "crc16.h"
}

proto1::proto1() : uart()
{
    connect(this, SIGNAL(messageReceived(buffer_t*)), this, SLOT(handleIncomming(buffer_t*)));
}

void proto1::handleIncomming(buffer_t *lastmsg){
    rx_msg rx_req;
    uint8_t* pl = new uint8_t[150];
    rx_req.payload = pl;

    if(cp_decodemessage(lastmsg->buffer, lastmsg->wrt_ptr - &lastmsg->buffer[0], &rx_req) == 0){

        if(rx_req.cmd == resource_count){
            emit reqResourceCount(&rx_req);
        }
        else if(rx_req.cmd == resource_config){
            emit reqConfig(&rx_req);
        }
        else if(rx_req.cmd == resource_value_update){
            emit reqValueUpdate(&rx_req);
        }
        else if(rx_req.cmd == resource_req_updateAll){
            emit reqValueUpdateAll(&rx_req);
        }
    }
}

int proto1::frameandtx(uint8_t *payload, uint8_t len, req_cmd cmd, uint8_t seqno){
    uint8_t buf[200];

    len = cp_encodemessage(seqno, cmd, payload, len, &buf[0]);
    transmit((const char*)&buf[0], len);

    return 0;
}

void testdecode(uint8_t* buffer){
    uint8_t id;
    uint8_t string[30];
    uint32_t len;

    cp_decodeID(buffer, &id, &len);
    cp_decodeReadings(buffer+len, &string[0], &len);
    qDebug() << "ID: " << id;
    qDebug() << "Value: " << QString::fromLatin1((const char*)string);

}

int proto1::frameandtx(uint8_t id, cmp_object_t* value, enum req_cmd cmd, uint8_t seqno){
    uint8_t buf[20];
    cmp_object_t obj;
    int len = 0;

    obj.type = CMP_TYPE_POSITIVE_FIXNUM;
    obj.as.u8 = id;
    len += cp_encodereading(&buf[0], &obj);
    len += cp_encodereading(&buf[len], value);

    frameandtx(&buf[0], len, cmd, seqno);
    testdecode(&buf[0]);
    return 0;
}

