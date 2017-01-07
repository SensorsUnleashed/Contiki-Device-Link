#include "node.h"

ledindicator::ledindicator(node* parent, QString uri, QVariantMap attributes) :
    sensor(parent, uri, attributes)
{

}

void ledindicator::toggleRedLED(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=10");
    put_request(pdu, req_setCommand, 0);
}

void ledindicator::toggleGreenLED(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=11");
    put_request(pdu, req_setCommand, 0);
}

void ledindicator::toggleOrangeLED(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=12");
    put_request(pdu, req_setCommand, 0);
}

void ledindicator::toggleYellowLED(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"setCommand=13");
    put_request(pdu, req_setCommand, 0);
}
