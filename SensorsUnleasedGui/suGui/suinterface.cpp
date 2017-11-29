#include "node.h"

suinterface::suinterface(QHostAddress addr) : wsn(addr){



}

uint16_t suinterface::get_request(CoapPDU *pdu, enum request req, QByteArray payload){
    msgid t;
    t.req = req;
    t.number = qrand();

    pdu->setType(CoapPDU::COAP_CONFIRMABLE);
    pdu->setCode(CoapPDU::COAP_GET);
    pdu->setToken((uint8_t*)&t.number,2);

    enum CoapPDU::ContentFormat ct = CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET;
    pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
    pdu->setMessageID(t.number);

    token.append(t);

    char tmp[30];
    int len;
    pdu->getURI(tmp,30, &len);

    send(pdu, t.number, payload);

    return t.number;
}

uint16_t suinterface::put_request(CoapPDU *pdu, enum request req, QByteArray payload){
    msgid t;
    t.req = req;
    t.number = qrand();

    pdu->setType(CoapPDU::COAP_CONFIRMABLE);
    pdu->setCode(CoapPDU::COAP_PUT);

    //Set the token, if it is not set beforehand
    if(pdu->getTokenLength() <= 0){
        pdu->setToken((uint8_t*)&t.number,2);
    }

    enum CoapPDU::ContentFormat ct = CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET;
    pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
    pdu->setMessageID(t.number);

    send(pdu, t.number, payload);

    token.append(t);

    return t.number;
}
