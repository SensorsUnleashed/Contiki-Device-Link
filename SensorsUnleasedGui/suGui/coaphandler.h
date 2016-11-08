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
    QByteArray uri;
    uint16_t token;  //The first messageid, used for finding the right message reply from the gui
    uint16_t messageid;     //The message id, used when quering the nodes. Will shift if message is split in multible chunks
    CoapPDU* lastPDU;    //The inital message send to the node
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

typedef enum {
    CA_BLOCK_SIZE_16_BYTE = 0,     /**< 16 byte */
    CA_BLOCK_SIZE_32_BYTE = 1,     /**< 32 byte */
    CA_BLOCK_SIZE_64_BYTE = 2,     /**< 64 byte */
    CA_BLOCK_SIZE_128_BYTE = 3,    /**< 128 byte */
    CA_BLOCK_SIZE_256_BYTE = 4,    /**< 256 byte */
    CA_BLOCK_SIZE_512_BYTE = 5,    /**< 512 byte */
    CA_BLOCK_SIZE_1_KBYTE = 6      /**< 1 Kbyte */
} CABlockSize_t;

struct s_statusstr{
    enum CoapPDU::Code code;
    const char* string;
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
    uint32_t prefMsgSize;
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
    int calc_block_option(uint8_t more, uint32_t num, uint32_t msgsize, uint8_t* blockval, uint16_t *len);

    void removePDU(uint16_t token);
    struct coapMessageStore* findPDU(CoapPDU *pdu);

    void parseMessage(QHostAddress sender, struct coapMessageStore* message);
    QVariant parseAppLinkFormat(QByteArray payload);

    QString codeTostring(enum CoapPDU::Code code);

    struct s_statusstr statusstr[28] = {
        {CoapPDU::COAP_EMPTY,                       "COAP_EMPTY"},
        {CoapPDU::COAP_GET,                         "COAP_GET"},
        {CoapPDU::COAP_POST,                        "COAP_POST"},
        {CoapPDU::COAP_PUT,                         "COAP_PUT"},
        {CoapPDU::COAP_DELETE,                      "COAP_DELETE"},
        {CoapPDU::COAP_LASTMETHOD,                  "COAP_LASTMETHOD"},
        {CoapPDU::COAP_CREATED,                     "COAP_CREATED"},
        {CoapPDU::COAP_DELETED,                     "COAP_DELETED"},
        {CoapPDU::COAP_VALID,                       "COAP_VALID"},
        {CoapPDU::COAP_CHANGED,                     "COAP_CHANGED"},
        {CoapPDU::COAP_CONTENT,                     "COAP_CONTENT"},
        {CoapPDU::COAP_BAD_REQUEST,                 "COAP_BAD_REQUEST"},
        {CoapPDU::COAP_UNAUTHORIZED,                "COAP_UNAUTHORIZED"},
        {CoapPDU::COAP_BAD_OPTION,                  "COAP_BAD_OPTION"},
        {CoapPDU::COAP_FORBIDDEN,                   "COAP_FORBIDDEN"},
        {CoapPDU::COAP_NOT_FOUND,                   "COAP_NOT_FOUND"},
        {CoapPDU::COAP_METHOD_NOT_ALLOWED,          "COAP_METHOD_NOT_ALLOWED"},
        {CoapPDU::COAP_NOT_ACCEPTABLE,              "COAP_NOT_ACCEPTABLE"},
        {CoapPDU::COAP_PRECONDITION_FAILED,         "COAP_PRECONDITION_FAILED"},
        {CoapPDU::COAP_REQUEST_ENTITY_TOO_LARGE,    "COAP_REQUEST_ENTITY_TOO_LARGE"},
        {CoapPDU::COAP_UNSUPPORTED_CONTENT_FORMAT,  "COAP_UNSUPPORTED_CONTENT_FORMAT"},
        {CoapPDU::COAP_INTERNAL_SERVER_ERROR,       "COAP_INTERNAL_SERVER_ERROR"},
        {CoapPDU::COAP_NOT_IMPLEMENTED,             "COAP_NOT_IMPLEMENTED"},
        {CoapPDU::COAP_BAD_GATEWAY,                 "COAP_BAD_GATEWAY"},
        {CoapPDU::COAP_SERVICE_UNAVAILABLE,         "COAP_SERVICE_UNAVAILABLE"},
        {CoapPDU::COAP_GATEWAY_TIMEOUT,             "COAP_GATEWAY_TIMEOUT"},
        {CoapPDU::COAP_PROXYING_NOT_SUPPORTED,      "COAP_PROXYING_NOT_SUPPORTED"},
        {CoapPDU::COAP_UNDEFINED_CODE,              "COAP_UNDEFINED_CODE"}
    };

private slots:
    void readPendingDatagrams();
    void timeout();

public slots:

signals:
    void coapMessageRdy(QVariant messageid);
    void coapRetrans(QVariant token, QVariant count, QVariant outof);
    void coapCode(QVariant token, QString code);
};

#endif // COAPHANDLER_H
