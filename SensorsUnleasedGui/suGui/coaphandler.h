#ifndef COAPHANDLER_H
#define COAPHANDLER_H

#include "database.h"
#include "cantcoap/cantcoap.h"
#include <QObject>
#include <QVariant>
#include <QMap>
#include <QHostAddress>
#include <QUdpSocket>
#include <QTimer>
#include <QElapsedTimer>

#define REST_MAX_CHUNK_SIZE            48

struct coapMessageStore{
    QHostAddress addr;
    uint16_t token;  //The first messageid, used for finding the right message reply from the gui
    uint16_t messageid;     //The message id, used when quering the nodes. Will shift if message is split in multible chunks
    CoapPDU* initialPDU;    //The inital message send to the node
    QByteArray rx_payload;  //The payload. Will be assembled as the right messages rolls in
    QByteArray tx_payload;
    uint32_t tx_next_index; //What should be send next
    uint32_t num;           //Active number (Not yet acknowledged)
    enum CoapPDU::ContentFormat ct; //How should the payload be parsed?
    QElapsedTimer txtime;
    uint8_t retranscount;
};

struct sunode{
    QHostAddress ip;
    QVariantMap links;
    QVariant recvMessage;
};

class coaphandler : public QObject
{
    Q_OBJECT
public:
    coaphandler(database* db);
    Q_INVOKABLE void updateSettings(QVariant setup);
    Q_INVOKABLE QVariant getSettings();
    Q_INVOKABLE QVariant getNodeLinks(QVariant nodeaddr);
    Q_INVOKABLE QVariant getNodeMessage(QVariant nodeaddr);

    Q_INVOKABLE QVariant reqGet(QVariant nodeaddr, QVariant uri, QVariant options, QVariant oldtoken, QByteArray payload=0);

private:
    QTimer* acktimer;
    database *db;
    QUdpSocket* udpSocket;
    QVector<struct sunode*> knownnodes;
    QVector<struct coapMessageStore*> activePDUs;
    uint16_t ackTimeout;
    uint16_t prefMsgSize;
    uint8_t retransmissions;

    struct sunode* findNode(QHostAddress addr);

    int parseQMLOptions(QVariant options,
                        enum CoapPDU::ContentFormat* ct,
                        enum CoapPDU::Type* type,
                        enum CoapPDU::Code* code);

    void send(QHostAddress addr, uint8_t *pduptr, int len);
    void initSocket();
    void printPDU(CoapPDU* pdu);
    CoapPDU::CoapOption* coap_check_option(CoapPDU *pdu, enum CoapPDU::Option opt);
    int parseBlockOption(CoapPDU::CoapOption *blockoption, uint8_t* more, uint32_t *num, uint8_t *SZX);
    int calc_block_option(uint8_t more, uint32_t num, uint16_t msgsize, uint8_t* blockval, uint8_t* len);

    void removePDU(uint16_t token);
    struct coapMessageStore* findPDU(CoapPDU *pdu);

    void parseMessage(QHostAddress sender, struct coapMessageStore* message);
    QVariant parseAppLinkFormat(QByteArray payload);

private slots:
    void readPendingDatagrams();
    void timeout();

public slots:

signals:
    void coapMessageRdy(QVariant messageid);
};

#endif // COAPHANDLER_H
