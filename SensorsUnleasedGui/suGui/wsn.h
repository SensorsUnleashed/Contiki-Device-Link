#ifndef WSN_H
#define WSN_H

#include <QObject>
#include <QDebug>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QTimer>
#include "cantcoap/cantcoap.h"

struct coapMessageStore_{
    uint16_t token;  //The first messageid, used for finding the right message reply from the gui
    CoapPDU* lastPDU;    //The inital message send to the node
    QByteArray rx_payload;  //The payload. Will be assembled as the right messages rolls in
    QByteArray tx_payload;
    uint32_t tx_next_index; //What should be send next
    uint32_t num;           //Active number (Not yet acknowledged)
    QElapsedTimer txtime;
    uint8_t retranscount;
};

class wsn : public QObject
{
    Q_OBJECT
public:
    explicit wsn(QHostAddress addr);

    void send(CoapPDU *pdu, uint16_t token, QByteArray payload=0);

    Q_INVOKABLE void stopListening();

    void parseData(QByteArray datagram);
    void parseMessage(coapMessageStore_* message, CoapPDU::Code code);

    virtual void nodeNotResponding(uint16_t token){ Q_UNUSED(token); qDebug() << "Implement this";}
    virtual QVariant parseTextPlainFormat(uint16_t token, QByteArray payload){ qDebug() << "wsn::parseTextPlainFormat " << payload << " token=" << token; return QVariant(0);}
    virtual QVariant parseAppLinkFormat(uint16_t token, QByteArray payload) { Q_UNUSED(payload); Q_UNUSED(token); qDebug() << "wsn::parseAppLinkFormat Implement this"; return QVariant(0);}
    virtual QVariant parseAppXmlFormat(uint16_t token, QByteArray payload) { Q_UNUSED(payload); Q_UNUSED(token); qDebug() << "wsn::parseAppXmlFormat Implement this"; return QVariant(0);}
    virtual QVariant parseAppOctetFormat(uint16_t token, QByteArray payload, CoapPDU::Code code) { Q_UNUSED(payload); Q_UNUSED(token); Q_UNUSED(code); qDebug() << "wsn::parseAppOctetFormat Implement this"; return QVariant(0);}
    virtual QVariant parseAppExiFormat(uint16_t token, QByteArray payload) { Q_UNUSED(payload); Q_UNUSED(token); qDebug() << "wsn::parseAppExiFormat Implement this"; return QVariant(0);}
    virtual QVariant parseAppJSonFormat(uint16_t token, QByteArray payload) { Q_UNUSED(payload); Q_UNUSED(token); qDebug() << "wsn::parseAppJSonFormat Implement this"; return QVariant(0);}
    virtual void handleReturnCode(uint16_t token, CoapPDU::Code code) { Q_UNUSED(token); Q_UNUSED(token); qDebug() << "wsn::handleReturnCode Implement this"; }

    void removePDU(uint16_t token);
    int calc_block_option(uint8_t more, uint32_t num, uint32_t msgsize, uint8_t* blockval, uint16_t* len);
    int parseBlockOption(CoapPDU::CoapOption* blockoption, uint8_t* more, uint32_t* num, uint8_t* SZX);
    CoapPDU::CoapOption* coap_check_option(CoapPDU *pdu, enum CoapPDU::Option opt);
    struct coapMessageStore_* findPDU(CoapPDU* pdu);

private:
    QHostAddress addr;

    uint32_t prefMsgSize;
    uint16_t ackTimeout;
    uint8_t retransmissions;
    QTimer* acktimer;

    QVector<struct coapMessageStore_*> activePDUs;
signals:
    void timeoutinfo(QVariant retransnumber, QVariant maxretries);
public slots:

private slots:
    void timeout();
};

#endif // WSN_H
