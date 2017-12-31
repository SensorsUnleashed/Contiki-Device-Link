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
#include "wsn.h"
#include "socket.h"

wsn::wsn(QHostAddress addr)
{
    this->addr = addr;
    prefMsgSize = 32;   //Should be taken from the db
    ackTimeout = 2000;
    retransmissions = 3;

    acktimer = new QTimer();
    acktimer->setSingleShot(true);
    connect(acktimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void wsn::send(CoapPDU *pdu, msgid cmdref, QByteArray payload){
    struct coapMessageStore_* storedPDU = new struct coapMessageStore_;

    /*Add the payload if there is any */
    if(!payload.isEmpty()){
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

    socket* conn = socket::getInstance();
    conn->send(addr, pdu->getPDUPointer(), pdu->getPDULength());

    conn->observe(this, addr);

    /* Only start listening and set the timeout timer if its a confirmable message */
    if(pdu->getType() == CoapPDU::COAP_CONFIRMABLE){
        if(!acktimer->isActive()){
            acktimer->start(ackTimeout);
        }
    }

    storedPDU->txtime.start();
    storedPDU->lastPDU = pdu;
    storedPDU->token = cmdref.number;
    storedPDU->tokenref = cmdref;
    storedPDU->retranscount = 0;
    storedPDU->keep = 0;
    activePDUs.append(storedPDU);

    /* Reset the progressbar */
    emit timeoutinfo(0, retransmissions);
}

void wsn::stopListening(){
    socket* conn = socket::getInstance();
    conn->observe_stop(this);
    qDebug() << "Stop listening for: " << addr.toString();
}

void wsn::timeout(){
    qDebug() << "Timeout";
    qint64 nexttimeout = -1;

    QVector<uint16_t> delindex;

    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->txtime.isValid()){
            if(activePDUs[i]->txtime.hasExpired(ackTimeout)){
                if(activePDUs[i]->retranscount >= retransmissions){ //Give up trying
                    qDebug() << "Giving up on message";
                    //Mark PDU for Deletion pdu
                    delindex.append(activePDUs[i]->token);
                    nodeNotResponding(activePDUs[i]->tokenref);
                }
                else{   //Try again
                    if(activePDUs[i]->lastPDU->getType() == CoapPDU::COAP_CONFIRMABLE){

                        socket* conn = socket::getInstance();
                        conn->send(addr, activePDUs[i]->lastPDU->getPDUPointer(), activePDUs[i]->lastPDU->getPDULength());
                        activePDUs[i]->retranscount++;
                        //reset timeout
                        activePDUs[i]->txtime.start();
                        if(nexttimeout == -1){
                            nexttimeout = ackTimeout;
                        }
                    }
                    else{
                        //It was a COAP_NON_CONFIRMABLE pdu. Remove it now
                        delindex.append(activePDUs[i]->token);
                    }
                }
                emit timeoutinfo(activePDUs[i]->retranscount, retransmissions);
            }
            else    //Not yet expired. Find next timeout - Wait at least 100ms
            {
                qint64 elabsed = activePDUs[i]->txtime.elapsed();
                nexttimeout = nexttimeout < elabsed ? elabsed : nexttimeout;
                nexttimeout = nexttimeout < 100 ? 100 : nexttimeout;
            }
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

void wsn::disableTokenRemoval(uint16_t token){
    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token){
            activePDUs[i]->txtime.invalidate();
            activePDUs[i]->keep = 1;
            break;
        }
    }
}

void wsn::enableTokenRemoval(uint16_t token){
    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token){
            activePDUs[i]->keep = 0;
            break;
        }
    }
}

void wsn::removePDU(uint16_t token){
    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token && !activePDUs[i]->keep){
            delete activePDUs[i]->lastPDU;
            delete activePDUs[i];
            activePDUs.remove(i);
            break;
        }
    }
}

int wsn::calc_block_option(uint8_t more, uint32_t num, uint32_t msgsize, uint8_t* blockval, uint16_t* len){
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

int wsn::parseBlockOption(CoapPDU::CoapOption* blockoption, uint8_t* more, uint32_t* num, uint8_t* SZX){
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

//Returns 0 if the option is not found,
//Returns a pointer to the option if found
CoapPDU::CoapOption* wsn::coap_check_option(CoapPDU *pdu, enum CoapPDU::Option opt){
    CoapPDU::CoapOption* options = pdu->getOptions();
    int len = pdu->getNumOptions();

    for(int i=0; i<len; i++){
        if((options+i)->optionNumber == opt){
            return options+i;
        }
    }
    return 0;
}

struct coapMessageStore_* wsn::findPDU(CoapPDU* pdu){
    uint16_t token;
    memcpy(&token, pdu->getTokenPointer(), pdu->getTokenLength());

    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token) return activePDUs[i];
    }
    return 0;
}

void wsn::send_RST(CoapPDU *recvPDU){
    CoapPDU *rstPDU = new CoapPDU();

    QByteArray uri(200, 0);
    int urilen;
    recvPDU->getURI(uri.data(), 200, &urilen);
    rstPDU->setURI(uri.data(), urilen);
    rstPDU->setMessageID(recvPDU->getMessageID());
    rstPDU->setToken(recvPDU->getTokenPointer(), recvPDU->getTokenLength());
    rstPDU->setType(CoapPDU::COAP_RESET);
    socket* conn = socket::getInstance();
    conn->send(addr, rstPDU->getPDUPointer(), rstPDU->getPDULength());
    delete rstPDU;
}

void wsn::send_ACK(CoapPDU *recvPDU){
    CoapPDU *ackPDU = new CoapPDU();

    QByteArray uri(200, 0);
    int urilen;
    ackPDU->getURI(uri.data(), 200, &urilen);
    ackPDU->setURI(uri.data(), urilen);
    ackPDU->setMessageID(recvPDU->getMessageID());
    ackPDU->setToken(recvPDU->getTokenPointer(), recvPDU->getTokenLength());
    ackPDU->setType(CoapPDU::COAP_ACKNOWLEDGEMENT);
    socket* conn = socket::getInstance();
    conn->send(addr, ackPDU->getPDUPointer(), ackPDU->getPDULength());
    delete ackPDU;
}

void wsn::parseData(QByteArray datagram){
    //processTheDatagram(datagram);
    CoapPDU *recvPDU = new CoapPDU((uint8_t*)datagram.data(),datagram.length());
    CoapPDU *txPDU; //Assign this pdu to the next pdu to send, and switch out with the one in the store
    CoapPDU::CoapOption* options = 0;
    int dotx = 0;

    if(recvPDU->validate()) {

        struct coapMessageStore_* storedPDUdata = findPDU(recvPDU);

        if(recvPDU->getType() == CoapPDU::COAP_ACKNOWLEDGEMENT){
            qDebug() << "its an ack";
        }
        //We expected this message - handle it
        if(storedPDUdata != 0){
            CoapPDU::Code code = recvPDU->getCode();
            qDebug() << "Code: " << code;

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
                        parseMessage(storedPDUdata, code);
                    }
                }
            }   //Block2 handling
            else{   //Received a plain single message, handle it and ack if needed
                if(recvPDU->getPayloadLength()){
                    uint8_t* pl = recvPDU->getPayloadPointer();
                    for(int i=0; i<recvPDU->getPayloadLength(); i++){
                        storedPDUdata->rx_payload[i] = *(pl+i);
                    }
                    //Handle single messages
                    parseMessage(storedPDUdata, code);

                    //Acknowledge it if needed
//                    if(recvPDU->getType() == CoapPDU::COAP_CONFIRMABLE){
//                        qDebug() << "send an ACK!";
//                        send_ACK(recvPDU);
//                    }
                }
            }

            //Do the transmitting part
            if(dotx){

                //Request the same content format, as it sends us
                options = coap_check_option(storedPDUdata->lastPDU, CoapPDU::COAP_OPTION_CONTENT_FORMAT);
                if(options){
                    if(options->optionValueLength > 0){
                        txPDU->setContentFormat((enum CoapPDU::ContentFormat)*options->optionValuePointer);
                    }
                }

                txPDU->setMessageID(recvPDU->getMessageID() + 1);
                txPDU->setToken(storedPDUdata->lastPDU->getTokenPointer(), storedPDUdata->lastPDU->getTokenLength());
                txPDU->setType(storedPDUdata->lastPDU->getType());
                txPDU->setCode(storedPDUdata->lastPDU->getCode());

                QByteArray uri(200, 0);
                int urilen;
                storedPDUdata->lastPDU->getURI(uri.data(), 200, &urilen);
                txPDU->setURI(uri.data(), urilen);

                //Switch out the old pdu with the new
                delete storedPDUdata->lastPDU;
                storedPDUdata->lastPDU = txPDU;

                socket* conn = socket::getInstance();
                conn->send(addr, txPDU->getPDUPointer(), txPDU->getPDULength());
            }
            else{
                handleReturnCode(storedPDUdata->tokenref, code);
                removePDU(storedPDUdata->token);
            }
        }
        else{   //Message unknown
            qDebug() << "send a RST!";
            //send_RST(recvPDU);
        }
    }

    delete recvPDU;
}

//Returns an acknowledment, if that is needed
QVariant wsn::parseMessage(coapMessageStore_* message, CoapPDU::Code code){
    QVariant ret(0);
    int ct = 0;
    CoapPDU::CoapOption* options = 0;
    options = coap_check_option(message->lastPDU, CoapPDU::COAP_OPTION_CONTENT_FORMAT);
    if(options){
        if(options->optionValueLength > 0){
            ct = (enum CoapPDU::ContentFormat)*options->optionValuePointer;
        }
    }
    else{
        qDebug() << "Unknown content format - unable to parse";
        qDebug() << message->rx_payload;
        return ret;
    }

    switch(ct){
    case CoapPDU::COAP_CONTENT_FORMAT_TEXT_PLAIN:
        parseTextPlainFormat(message->tokenref, message->rx_payload);
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_LINK:
        parseAppLinkFormat(message->tokenref, message->rx_payload);
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_XML:
        parseAppXmlFormat(message->tokenref, message->rx_payload);
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_XML";
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET:
        ret = parseAppOctetFormat(message->tokenref, message->rx_payload, code);
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_EXI:
        parseAppExiFormat(message->tokenref, message->rx_payload);
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_EXI";
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_JSON:
        parseAppJSonFormat(message->tokenref, message->rx_payload);
        qDebug() << "CoapPDU::COAP_CONTENT_FORMAT_APP_JSON";
        break;
    }
    return ret;
}
