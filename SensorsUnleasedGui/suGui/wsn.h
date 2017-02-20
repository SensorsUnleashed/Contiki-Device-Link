/*******************************************************************************
 * Copyright (c) 2017, Ole Nissen.
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met: 
 *  1. Redistributions of source code must retain the above copyright 
 *  notice, this list of conditions and the following disclaimer. 
 *  2. Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following
 *  disclaimer in the documentation and/or other materials provided
 *  with the distribution. 
 *  3. The name of the author may not be used to endorse or promote
 *  products derived from this software without specific prior
 *  written permission.  
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 * This file is part of the Sensors Unleashed project
 *******************************************************************************/
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
