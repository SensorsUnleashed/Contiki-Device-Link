#include "coaphandler.h"
#include <QVariant>
#include <QHostAddress>
#include <QDebug>

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
        }
    }
    else{
        qDebug() << "Database not working - use default values!";
        ackTimeout = 5;
        prefMsgSize = 64;
    }

    initSocket();
}

void coaphandler::updateSettings(QVariant setup){
    QVariantMap map = setup.toMap();

    ackTimeout = map["coapAckTimeout"].toInt();
    prefMsgSize = map["coapPrefMsgSize"].toInt();

    //Update the sql database
    QVariantList data;
    QVariantMap item;
    item["Key"] = "ackTimeout";
    item["Value"] = ackTimeout;
    data.insert(0, item);

    item["Key"] = "prefMsgSize";
    item["Value"] = prefMsgSize;
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
        pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);    //App xml
        pdu->setMessageID(ret);

        if(payload.size() > 0){
            if(payload.length() > prefMsgSize){  //Payload needs to be split
                uint8_t buf[3];
                uint8_t len;
                calc_block_option(1, 0, prefMsgSize, &buf[0], &len);
                pdu->addOption(CoapPDU::COAP_OPTION_BLOCK1, len, &buf[0]);
                pdu->setPayload((uint8_t*)payload.data(), prefMsgSize);
                storedPDU->tx_payload = payload;
                storedPDU->tx_next_index = prefMsgSize;
            }
            else{   //Normal single message payload
                pdu->setPayload((uint8_t*)payload.data(), payload.length());
            }
        }

        char* uristring = uri.toString().toLatin1().data();
        pdu->setURI(uristring, strlen(uristring));
        send(addr, pdu->getPDUPointer(), pdu->getPDULength());

        storedPDU->initialPDU = pdu;
        storedPDU->messageid = ret;
        storedPDU->token = ret;
        storedPDU->ct = ct; //We assume we receive what we request
        activePDUs.append(storedPDU);
        printPDU(pdu);
    }
    else{
        qDebug() << "Error in IPv6 address" << nodeaddr.toString();
    }

    return QVariant(ret);
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

void coaphandler::send(QHostAddress addr, uint8_t* pduptr, int len){
    qDebug() << "Send pdu to: " << addr.toString();
    udpSocket->writeDatagram((char*)pduptr, len, addr, 5683);

    //    QByteArray tx;
    //    tx.resize(len);
    //    memcpy(tx.data(), pduptr, len);

    //    QString valueInHex;
    //    for(int i=0; i<tx.length(); i++){

    //        valueInHex += QString("0x%1 ").arg((quint8)tx[i] , 0, 16);
    //    }
    //    qDebug() << "TX: " << valueInHex;
}

void coaphandler::initSocket()
{
    udpSocket = new QUdpSocket(this);
    if(udpSocket->bind(QHostAddress::AnyIPv6, 5683)){
        qDebug() << "Successfully bound to Localhost port 5683";
    }

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
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

int coaphandler::calc_block_option(uint8_t more, uint32_t num, uint16_t msgsize, uint8_t* blockval, uint8_t* len){
    //SSS.M.NNNN.NNNN.NNNN.NNNN.NNNN
    //S = SZX = exponetial 4-10
    //M = More
    //N = NUM

    uint16_t szx = msgsize;
    for(int i=0; i<10; i++){
        if((szx >> i) & 0x1){
            szx = i;
            break;
        }
    }

    *(blockval + 0) = 0;
    *(blockval + 0) = szx;
    *(blockval + 0) |= more << 3;
    *(blockval + 0) |= num << 4;
    *(blockval + 1) = num >> 4;
    *(blockval + 2) = num >> 12;

    if(msgsize < 16){
        *len = 1;
    }
    else if(msgsize < 4096){
        *len = 2;
    }
    else{
        *len = 3;
    }

    return 0;
}


void coaphandler::printPDU(CoapPDU* pdu){
#if 0
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

    uint8_t* tokenptr = pdu->getTokenPointer();
    int tokenlen = pdu->getTokenLength();
    memcpy(tokenbuf.data(), tokenptr, tokenlen);
    tokenbuf.resize(tokenlen);
    qDebug() << "Token: " << QString(tokenbuf);
    qDebug() << "MessageID: " << pdu->getMessageID();

    CoapPDU::CoapOption* options = pdu->getOptions();
    for(int i=0; i<pdu->getNumOptions(); i++){
        qDebug() << (options+i)->optionNumber;
        QByteArray opt;
        opt.resize((options+i)->optionValueLength);
        memcpy(opt.data(), (options+i)->optionValuePointer, opt.size());
        qDebug() << "Optionval: " << QString(opt);

        if((options+i)->optionNumber == CoapPDU::COAP_OPTION_BLOCK2){
            uint32_t offset;
            uint8_t more;
            uint32_t num;
            getBlockOffset((options+i), &offset, &more, &num );
            qDebug() << "Offset=" << offset << " more=" << more << " num=" << num;
        }
    }

    qDebug() << "--------------------";
#endif
}
void coaphandler::removePDU(uint16_t token){
    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token){
            activePDUs.remove(i);
            break;
        }
    }
}

struct coapMessageStore* coaphandler::findPDU(CoapPDU* pdu){
    uint16_t token;
    memcpy(&token, pdu->getTokenPointer(), pdu->getTokenLength());
    qDebug() << "Token = " << token;

    for(int i=0; i<activePDUs.count(); i++){
        if(activePDUs[i]->token == token) return activePDUs[i];
    }
    return 0;
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
        CoapPDU::CoapOption* options = 0;
        if(recvPDU->validate()) {
            printPDU(recvPDU);

            struct coapMessageStore* storedPDU = findPDU(recvPDU);
            //We expected this message - handle it
            if(storedPDU != 0){
                options = coap_check_option(recvPDU, CoapPDU::COAP_OPTION_CONTENT_FORMAT);
                if(options){
                    if(options->optionValueLength > 0){
                        storedPDU->ct = (enum CoapPDU::ContentFormat)*options->optionValuePointer;
                    }
                }

                /* Handle block1 - response from a put/post (Send more money) */
                options = coap_check_option(recvPDU, CoapPDU::COAP_OPTION_BLOCK1);
                if(options != 0){
                    uint8_t more;
                    uint32_t num;
                    uint8_t szx;
                    if(parseBlockOption(options, &more, &num, &szx) == 0){
                        uint32_t prefsize = 1 << (szx + 4);
                        uint32_t bytesleft = storedPDU->tx_payload.length() - storedPDU->tx_next_index;

                        if(bytesleft){
                            uint8_t buf[3];
                            uint8_t len;

                            more = bytesleft > prefsize;
                            calc_block_option(more, ++num, prefsize, &buf[0], &len);
                            storedPDU->initialPDU->addOption(CoapPDU::COAP_OPTION_BLOCK1, len, &buf[0]);
                            if(more){
                                storedPDU->initialPDU->setPayload((uint8_t*)(storedPDU->tx_payload.data()+storedPDU->tx_next_index), prefsize);
                                storedPDU->tx_next_index += prefsize;
                            }
                            else{
                                storedPDU->initialPDU->setPayload((uint8_t*)(storedPDU->tx_payload.data()+storedPDU->tx_next_index), bytesleft);
                                storedPDU->tx_next_index += bytesleft;
                            }

                            dotx = 1;
                        }
                        else{
                            qDebug() << "ACK - Finished transmitting large message";
                        }
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

                        uint8_t* pl = recvPDU->getPayloadPointer();
                        for(int i=0; i<recvPDU->getPayloadLength(); i++){
                            storedPDU->rx_payload[offset + i] = *(pl+i);
                        }
                        if(more){
                            qDebug() << "Received " << num + 1 << "messages, so far";
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
                            storedPDU->initialPDU->addOption(CoapPDU::COAP_OPTION_BLOCK2, valuelen, value);
                            dotx = 1;
                        }
                        else{
                            parseMessage(sender, storedPDU);
                        }
                    }
                }   //Block2 handling
                else{
                    uint8_t* pl = recvPDU->getPayloadPointer();
                    for(int i=0; i<recvPDU->getPayloadLength(); i++){
                        storedPDU->rx_payload[i] = *(pl+i);
                    }
                    //Handle single messages
                    parseMessage(sender, storedPDU);
                }

                if(dotx){
                    storedPDU->messageid++;
                    storedPDU->initialPDU->setMessageID(storedPDU->messageid);

                    printPDU(storedPDU->initialPDU);
                    send(sender, storedPDU->initialPDU->getPDUPointer(), storedPDU->initialPDU->getPDULength());
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

    switch(message->ct){
    case CoapPDU::COAP_CONTENT_FORMAT_TEXT_PLAIN:
        qDebug() << "Parse Plain text format";
        node->recvMessage = QString(message->rx_payload);
        qDebug() << node->recvMessage.toString();
        emit coapMessageRdy(QVariant(message->token));
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_LINK:
        qDebug() << "Parse APP Link message";
        ret = parseAppLinkFormat(message->rx_payload);
        node->links = ret.toMap();

        qDebug() << "Event AppLinkListRdy fired! id:" << message->token;
        emit coapMessageRdy(QVariant(message->token));
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_XML:
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET:
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_EXI:
        break;
    case CoapPDU::COAP_CONTENT_FORMAT_APP_JSON:
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


