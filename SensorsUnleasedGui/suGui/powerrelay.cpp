#include "node.h"

powerrelay::powerrelay(node* parent, QString uri, QVariantMap attributes, sensorstore *p) :
    sensor(parent, uri, attributes, p){

}

/* Move this to the relay class */
void powerrelay::toggleRelay(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=3");
    put_request(pdu, req_setCommand, 0);
}

void powerrelay::setOn(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=2");
    put_request(pdu, req_setCommand, 0);
}

void powerrelay::setOff(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=1");
    put_request(pdu, req_setCommand, 0);
}
