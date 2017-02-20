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
#include "coaphandler.h"
#include <QVariant>
#include <QHostAddress>
#include <QDebug>

#define DEBUG
#undef DEBUG

/**************************************************
 * Constructors/Deconstructors
***************************************************/

coaphandler::coaphandler(database *db)
{

    this->db = db;
    QString querystring = "select * from coap;";
    QVariantList result;

    //Read settings from the database
    if(db->query(querystring, &result) == 0){
        for(int i=0; i<result.count(); i++){
            QVariantMap keyval = result.at(i).toMap();
            if(keyval["Key"].toString().compare("ackTimeout") == 0){
                ackTimeout = keyval["Value"].toInt();
            }
            else if(keyval["Key"].toString().compare("prefMsgSize") == 0){
                prefMsgSize = keyval["Value"].toInt();
            }
            else if(keyval["Key"].toString().compare("retransmissions") == 0){
                retransmissions = keyval["Value"].toInt();
            }
        }
    }
    else{
        qDebug() << "Database not working - use default values!";
        ackTimeout = 5;
        prefMsgSize = 64;
        retransmissions = 3;
    }

    acktimer = new QTimer();
    acktimer->setSingleShot(true);
    connect(acktimer, SIGNAL(timeout()), this, SLOT(timeout()));
    //initSocket();
}

/**************************************************
 * Get/Set functions for the coaphandler
***************************************************/

void coaphandler::updateSettings(QVariant setup){
    QVariantMap map = setup.toMap();

    ackTimeout = map["coapAckTimeout"].toInt();
    prefMsgSize = map["coapPrefMsgSize"].toInt();
    retransmissions = map["coapRetrans"].toInt();

    //Update the sql database
    QVariantList data;
    QVariantMap item;
    item["Key"] = "ackTimeout";
    item["Value"] = ackTimeout;
    data.insert(0, item);

    item["Key"] = "prefMsgSize";
    item["Value"] = prefMsgSize;
    data.insert(1, item);

    item["Key"] = "retransmissions";
    item["Value"] = retransmissions;
    data.insert(1, item);

    db->update(QString("coap"), data);
}

QVariant coaphandler::getSettings(){
    QVariantMap map;
    map["coapAckTimeout"] = ackTimeout;

    uint16_t temp = prefMsgSize;
    for(int i=0; i<10; i++){
        if((temp >> i) & 0x1){
            temp = i;
            break;
        }
    }
    map["coapPrefMsgSize"] = temp;
    map["coapRetrans"] = retransmissions;
    return map;
}

QVariant coaphandler::getNodeLinks(QVariant nodeaddr){
    QHostAddress addr;
    struct sunode* node;
    if(addr.setAddress(nodeaddr.toString())){
        node = findNode(addr);
        if(node != 0){
            return node->links;
        }
    }
    return QVariant();
}

QVariant coaphandler::getNodeMessage(QVariant nodeaddr){
    QHostAddress addr;
    struct sunode* node;
    if(addr.setAddress(nodeaddr.toString())){
        node = findNode(addr);
        if(node != 0){
            return node->recvMessage;
        }
    }
    return QVariant();
}

/**************************************************
 * Public Action entry functions for the coaphandler
***************************************************/

/*
 * nodeaddr: The IpV6 address as a string
 * uri: The identifier for the resource published by the node
 * options: Contains the options used for the actual pdu
 * payload: The data to send to the node
 * oldtoken: Old tokenid. E.g. if a user hits a discover button, but he already requested a discovery. Delete the first request, and send a new.
 *
 * Returns the token, that is used for the transmission. The user should wait for this token to arrive.
*/
QVariant coaphandler::reqGet(QVariant nodeaddr, QVariant uri, QVariant options, QVariant oldtoken, QByteArray payload){
    uint16_t ret = 0;

    if(oldtoken.toInt() != -1){
        removePDU(oldtoken.toInt());
    }

    enum CoapPDU::ContentFormat ct;
    enum CoapPDU::Type type;
    enum CoapPDU::Code code;
    if(parseQMLOptions(options, &ct, &type, &code) > 0) return QVariant(-1);

    struct sunode* node;
    QHostAddress addr;
    if(addr.setAddress(nodeaddr.toString())){
        struct coapMessageStore* storedPDU = new struct coapMessageStore;
        node = findNode(addr);
        if(node == 0){
            node = new struct sunode;
            node->ip = addr;
            knownnodes.append(node);
        }

        ret = qrand();
        CoapPDU *pdu = new CoapPDU();
        pdu->setType(type);
        pdu->setCode(code);
        pdu->setToken((uint8_t*)&ret,2);
        pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
        pdu->setMessageID(ret);

        if(payload.size() > 0){
            if(payload.length() > (int)prefMsgSize){  //Payload needs to be split
                uint8_t buf[3];
                uint16_t len;
                calc_block_option(1, 0, prefMsgSize, &buf[0], &len);
                pdu->addOption(CoapPDU::COAP_OPTION_BLOCK1, len, &buf[0]);
                pdu->setPayload((uint8_t*)payload.data(), prefMsgSize);
                storedPDU->tx_payload = payload;
                storedPDU->tx_next_index = prefMsgSize;
                storedPDU->num = 0;
            }
            else{   //Normal single message payload
                pdu->setPayload((uint8_t*)payload.data(), payload.length());
            }
        }

        char* uristring = uri.toString().toLatin1().data();
        pdu->setURI(uristring, strlen(uristring));

        send(addr, pdu->getPDUPointer(), pdu->getPDULength());

        if(!acktimer->isActive()){
            acktimer->start(ackTimeout);
        }
        storedPDU->uri = uri.toByteArray();
        storedPDU->txtime.start();
        storedPDU->lastPDU = pdu;
        storedPDU->messageid = ret;
        storedPDU->token = ret;
        storedPDU->ct = ct;
        storedPDU->addr = addr;
        storedPDU->retranscount = 0;
        activePDUs.append(storedPDU);

        emit coapRetrans(QVariant(storedPDU->token), QVariant(storedPDU->retranscount), QVariant(retransmissions));
    }
    else{
        qDebug() << "Error in IPv6 address" << nodeaddr.toString();
    }

    qDebug() << "Token " << ret << " active";
    return QVariant(ret);
}

/**************************************************
 * Private functions for the coaphandler
***************************************************/

void coaphandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    if(udpSocket->bind(QHostAddress::AnyIPv6, 5683)){
        qDebug() << "Successfully bound to Localhost port 5683";
    }

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

void coaphandler::send(QHostAddress addr, uint8_t* pduptr, int len){
    //qDebug() << "Send pdu to: " << addr.toString();
    udpSocket->writeDatagram((char*)pduptr, len, addr, 5683);
}

void coaphandler::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        int dotx = 0;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        qDebug() << "Message received";
        //processTheDatagram(datagram);
        CoapPDU *recvPDU = new CoapPDU((uint8_t*)datagram.data(),datagram.length());
        CoapPDU *txPDU; //Assign this pdu to the next pdu to send, and switch out with the one in the store
        CoapPDU::CoapOption* options = 0;
        if(recvPDU->validate()) {
            printPDU(recvPDU);

            struct coapMessageStore* storedPDUdata = findPDU(recvPDU);

            //We expected this message - handle it
            if(storedPDUdata != 0){
                options = coap_check_option(recvPDU, CoapPDU::COAP_OPTION_CONTENT_FORMAT);
                if(options){
                    if(options->optionValueLength > 0){
                        storedPDUdata->ct = (enum CoapPDU::ContentFormat)*options->optionValuePointer;
                    }
                }

                CoapPDU::Code code = recvPDU->getCode();
                qDebug() << code;

                /* Handle block1 - response from a put/post (Send more money) */
                options = coap_check_option(recvPDU, CoapPDU::COAP_OPTION_BLOCK1);
                if(options != 0){
                    uint8_t more;
                    uint32_t num;
                    uint8_t szx;
                    uint32_t prefsize;
                    uint32_t bytesleft = storedPDUdata->tx_payload.length() - storedPDUdata->tx_next_index;

                    //Sender send us some options to use from now on
                    if(parseBlockOption(options, &more, &num, &szx) == 0){
                        prefsize = 1 << (szx + 4);
                        qDebug() << "Block1: " << num << "/" << more << "/" << prefsize;
                    }
                    else{   //We continue with whatever options we started with
                        num = storedPDUdata->num;
                        prefsize = prefMsgSize;
                        qDebug() << "Block1: " << num << "/" << more << "/" << prefsize << " else";
                    }

                    if(bytesleft){
                        //We need to send yet another block
                        txPDU = new CoapPDU();

                        uint8_t buf[3];
                        uint16_t len;
                        uint8_t* bufptr = &buf[0];

                        more = bytesleft > prefsize;
                        calc_block_option(more, ++num, prefsize, bufptr, &len);

                        txPDU->addOption(CoapPDU::COAP_OPTION_BLOCK1, len, bufptr);

                        storedPDUdata->num = num;
                        if(more){
                            txPDU->setPayload((uint8_t*)(storedPDUdata->tx_payload.data()+storedPDUdata->tx_next_index), prefsize);
                            storedPDUdata->tx_next_index += prefsize;

                        }
                        else{
                            txPDU->setPayload((uint8_t*)(storedPDUdata->tx_payload.data()+storedPDUdata->tx_next_index), bytesleft);
                            storedPDUdata->tx_next_index += bytesleft;
                        }

                        dotx = 1;
                    }
                    else{
                        qDebug() << "ACK - Finished transmitting large message";
                        emit coapCode(QVariant(storedPDUdata->token), codeTostring(code));
                        removePDU(storedPDUdata->token);
                    }
                }

                //Handle block2 - response to a get
                options = coap_check_option(recvPDU, CoapPDU::COAP_OPTION_BLOCK2);
                if(options != 0){
                    uint8_t more;
                    uint32_t num;
                    uint8_t szx;
                    if(parseBlockOption(options, &more, &num, &szx) == 0){
                        uint32_t offset = num << (szx + 4);

                        qDebug() << "Block2: " << num << "/" << more << "/" << (1 << (szx + 4));

                        uint8_t* pl = recvPDU->getPayloadPointer();
                        for(int i=0; i<recvPDU->getPayloadLength(); i++){
                            storedPDUdata->rx_payload[offset + i] = *(pl+i);
                        }
                        if(more){
                            //qDebug() << "Received " << num + 1 << "messages, so far";
                            uint8_t* value = options->optionValuePointer;
                            uint8_t valuelen = options->optionValueLength;
                            ++num;

                            *value &= 0x7;
                            *value |= num << 4;
                            //Clear all but the SXZ part
                            for(int i=1; i<valuelen; i++){
                                *(value+i)= 0;
                                *value |= (num << (i * 8 + 4));
                            }

                            txPDU = new CoapPDU();
                            txPDU->addOption(CoapPDU::COAP_OPTION_BLOCK2, valuelen, value);
                            dotx = 1;
                        }
                        else{
                            parseMessage(sender, storedPDUdata);
                            emit coapCode(QVariant(storedPDUdata->token), codeTostring(code));
                            removePDU(storedPDUdata->token);
                        }
                    }
                }   //Block2 handling
                else{   //Just a plain single message
                    if(recvPDU->getPayloadLength()){
                        uint8_t* pl = recvPDU->getPayloadPointer();
                        for(int i=0; i<recvPDU->getPayloadLength(); i++){
                            storedPDUdata->rx_payload[i] = *(pl+i);
                        }
                        //Handle single messages
                        parseMessage(sender, storedPDUdata);
                        emit coapCode(QVariant(storedPDUdata->token), codeTostring(code));
                        removePDU(storedPDUdata->token);
                    }

                }

                if(dotx){
                    storedPDUdata->messageid++;
                    txPDU->setURI(storedPDUdata->uri.data());
                    txPDU->setMessageID(storedPDUdata->messageid);
                    txPDU->setToken(storedPDUdata->lastPDU->getTokenPointer(), storedPDUdata->lastPDU->getTokenLength());
                    txPDU->setContentFormat(storedPDUdata->ct);
                    txPDU->setType(storedPDUdata->lastPDU->getType());
                    txPDU->setCode(storedPDUdata->lastPDU->getCode());

                    //Switch out the old pdu with the new
                    delete storedPDUdata->lastPDU;
                    storedPDUdata->lastPDU = txPDU;

                    printPDU(txPDU);
                    send(sender, txPDU->getPDUPointer(),txPDU->getPDULength());
                }
            }
        }
        delete recvPDU;
    }
}

void coaphandler::parseMessage(QHostAddress sender, coapMessageStore* message){

    //QMap parsingOutput;
    QVariant ret;
    struct sunode* node = findNode(sender);

    qDebug() << message->rx_payload;

    switch(message->ct){
    case CoapPDU::COAP_CONTENT_FORMAT_TEXT_PLAIN:
        node->recvMessage = QString(message->rx_payload);
        emit coapMessageRdy(QVariant(message->token));
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_LINK:
        ret = parseAppLinkFormat(message->rx_payload);
        node->links = ret.toMap();
        emit coapMessageRdy(QVariant(message->token));
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_XML:
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_XML";
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET:
        //This is the content format Sensors Unleashed uses for its data
        //emit coapSUMessageRdy(message->token, )
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET";
        emit coapMessageRdy(QVariant(message->token));
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_EXI:
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_EXI";
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_JSON:
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_JSON";
        break;
    }
}

QVariant coaphandler::parseAppLinkFormat(QByteArray payload){
    QVariantMap linklist;

    QString pl = QString(payload);
    //All resources are separeted by a ','
    QStringList rlist = pl.split(',', QString::SkipEmptyParts);
    //qDebug() << rlist;
    for(int i=0; i<rlist.count(); i++){
        QStringList slist = rlist.at(i).split(';', QString::SkipEmptyParts);
        QString root;

        //For now index 0 is always the uri
        root = slist.at(0);
        root.remove(QRegExp("[<>]"));
        if( root.at(0) == '/' ) root.remove( 0, 1 );    //Remove leading "/"
        slist.removeAt(0);
        linklist.insert(root, slist);
    }

    return linklist;
}

/**************************************************
 * Private timeout handler functions used by the coaphandler
***************************************************/
void coaphandler::timeout(){
    qint64 nexttimeout = -1;

    QVector<uint16_t> delindex;

    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->txtime.hasExpired(ackTimeout)){
            if(activePDUs[i]->retranscount >= retransmissions){ //Give up trying
                qDebug() << "Giving up on message";
                //Mark PDU for Deletion pdu
                delindex.append(activePDUs[i]->token);
            }
            else{   //Try again
                if(activePDUs[i]->lastPDU->getType() == CoapPDU::COAP_CONFIRMABLE){
                    send(activePDUs[i]->addr, activePDUs[i]->lastPDU->getPDUPointer(), activePDUs[i]->lastPDU->getPDULength());
                    activePDUs[i]->retranscount++;
                    //reset timeout
                    activePDUs[i]->txtime.start();
                    if(nexttimeout == -1){
                        nexttimeout = ackTimeout;
                    }
                    emit coapRetrans(QVariant(activePDUs[i]->token), QVariant(activePDUs[i]->retranscount), QVariant(retransmissions));
                }
                else{
                    //It was a COAP_NON_CONFIRMABLE pdu. Remove it now
                    delindex.append(activePDUs[i]->token);
                }
            }
        }
        else    //Not yet expired. Find next timeout - Wait at least 100ms
        {
            qint64 elabsed = activePDUs[i]->txtime.elapsed();
            nexttimeout = nexttimeout < elabsed ? elabsed : nexttimeout;
            nexttimeout = nexttimeout < 100 ? 100 : nexttimeout;
        }
    }

    if(nexttimeout > 0){
        acktimer->start(nexttimeout);
    }

    //Now cleanup
    for(int i=0; i<delindex.count(); i++){
        removePDU(delindex.at(i));
        qDebug() << "Deleted token " << delindex.at(i);
    }
}

/**************************************************
 * Helper functions used by the coaphandler
***************************************************/

//Return 0 for ok
//Return > 0 for error
int coaphandler::parseQMLOptions(QVariant options,
                                 enum CoapPDU::ContentFormat* ct,
                                 enum CoapPDU::Type* type,
                                 enum CoapPDU::Code* code
                                 ){
    QVariantMap map = options.toMap();

    bool ok;
    *ct = (enum CoapPDU::ContentFormat) map["ct"].toUInt(&ok);  if(!ok) return 1;
    *type = (enum CoapPDU::Type) map["type"].toUInt(&ok);       if(!ok) return 2;
    *code = (enum CoapPDU::Code) map["code"].toUInt(&ok);       if(!ok) return 3;


    return 0;
}

//Return a pointer to the node if its known or
//0 if not known
struct sunode* coaphandler::findNode(QHostAddress addr){

    for(int i=0; i<knownnodes.count(); i++){
        if(knownnodes[i]->ip == addr){
            return knownnodes[i];
        }
    }

    return 0;
}

//Returns 0 if the option is not found,
//Returns a pointer to the option if found
CoapPDU::CoapOption* coaphandler::coap_check_option(CoapPDU *pdu, enum CoapPDU::Option opt){
    CoapPDU::CoapOption* options = pdu->getOptions();
    int len = pdu->getNumOptions();

    for(int i=0; i<len; i++){
        if((options+i)->optionNumber == opt){
            return options+i;
        }
    }
    return 0;
}

//Return: 0 for ok
// 1 for error
int coaphandler::parseBlockOption(CoapPDU::CoapOption* blockoption, uint8_t* more, uint32_t* num, uint8_t* SZX){
    int len = blockoption->optionValueLength;
    uint8_t* value = blockoption->optionValuePointer;
    if(len >= 1){
        *more = (*value & 0x8) > 0;
        *SZX = (*value & 0x3);
        *num = *value >> 4;
        for(int j=1; j<len; j++){
            value++;
            *num |= ((uint32_t)(*value)) << (j * 8 + 4);
        }
    }
    else{
        return 1;
    }
    return 0;
}

int coaphandler::calc_block_option(uint8_t more, uint32_t num, uint32_t msgsize, uint8_t* blockval, uint16_t* len){
    /*
        We store in little, and let cantcoap send it big endian
        Illustration is in big endian.
           0
           0 1 2 3 4 5 6 7
          +-+-+-+-+-+-+-+-+
          |  NUM  |M| SZX |
          +-+-+-+-+-+-+-+-+

           0                   1
           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |          NUM          |M| SZX |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

           0               1                   2
           0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
          |                   NUM                 |M| SZX |
          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    SZX = exponetial 4-10 (16 - 1024 bytes)
    More = Will further blocks follow this
    NUM = Current block number (0 is the first)
*/

    uint32_t result = 0;

    //Calculate the exponential part
    uint16_t szx = msgsize >> 5;

    if (num < 16)
        *len = 1;
    else if(num < 4096)
        *len = 2;
    else
        *len = 3;

    result |= num;
    result <<= 4;
    result |= szx + (more << 3);

    memcpy(blockval, &result, *len);

    return 0;
}

void coaphandler::removePDU(uint16_t token){
    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token){
            delete activePDUs[i]->lastPDU;
            delete activePDUs[i];
            activePDUs.remove(i);
            break;
        }
    }
}

struct coapMessageStore* coaphandler::findPDU(CoapPDU* pdu){
    uint16_t token;
    memcpy(&token, pdu->getTokenPointer(), pdu->getTokenLength());

    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token) return activePDUs[i];
    }
    return 0;
}

QString coaphandler::codeTostring(enum CoapPDU::Code code){
    for(unsigned int i=0; i<sizeof(statusstr); i++){
        if(code == statusstr[i].code){
            return QString(statusstr[i].string);
        }
    }
    return "Unknown code " + QString::number((uint8_t)code);
}

void coaphandler::printPDU(CoapPDU* pdu){
#ifdef DEBUG
    QByteArray uribuf;
    uribuf.resize(100);
    int outlen;
    pdu->getURI(uribuf.data(), uribuf.length(), &outlen);
    uribuf.resize(outlen);
    qDebug() << "--------------------";
    qDebug() << "URI: " << QString(uribuf);
    qDebug() << "Code: " << pdu->getCode();
    qDebug() << "Type: " << pdu->getType();

    QByteArray tokenbuf;
    tokenbuf.resize(100);

    uint16_t token;
    memcpy(&token, pdu->getTokenPointer(), pdu->getTokenLength());
    qDebug() << "Token: " << token;
    qDebug() << "MessageID: " << pdu->getMessageID();

    CoapPDU::CoapOption* options = pdu->getOptions();
    for(int i=0; i<pdu->getNumOptions(); i++){
        qDebug() << (options+i)->optionNumber;
        QByteArray opt;
        opt.resize((options+i)->optionValueLength);
        memcpy(opt.data(), (options+i)->optionValuePointer, opt.size());
        qDebug() << "Optionval: " << QString(opt);

        if((options+i)->optionNumber == CoapPDU::COAP_OPTION_BLOCK2){
            uint8_t szx;
            uint8_t more;
            uint32_t num;
            parseBlockOption((options+i), &more, &num, &szx);
            qDebug() << "szx=" << szx << " more=" << more << " num=" << num;
        }
    }

    qDebug() << "Payload length: " << pdu->getPayloadLength();
    qDebug() << "--------------------";
#else
    Q_UNUSED(pdu);
#endif
}
