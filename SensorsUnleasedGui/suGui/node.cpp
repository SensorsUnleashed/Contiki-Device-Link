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
#include "node.h"
#include "socket.h"
//Created from QML
node::node(QHostAddress addr, QVariantMap data, sensorstore *p) : wsn(addr)
{
    ip = addr;
    databaseinfo = data;
    allsensorslist = p;
    ownsensorslist = new sensorstore;
    qDebug() << "Node: " << ip << " created";
}

sensor* node::getSensor(QString addr){
    for(int i=0; i<sensors.count(); i++){
        if(sensors.at(i)->getUri().compare(addr) == 0){
            return sensors.at(i);
        }
    }
    return 0;
}

void node::getSensorslist(){
    for(int i=0; i<sensors.count(); i++){
        emit sensorFound(sensors.at(i)->getUri(), sensors.at(i)->getClassType());
    }
}

/* Request the list of sensors from the node */
void node::requestLinks(){
    token = qrand();
    const char* uristring = ".well-known/core";

    CoapPDU *pdu = new CoapPDU();
    pdu->setType(CoapPDU::COAP_CONFIRMABLE);
    pdu->setCode(CoapPDU::COAP_GET);
    pdu->setToken((uint8_t*)&token,2);

    enum CoapPDU::ContentFormat ct = CoapPDU::COAP_CONTENT_FORMAT_APP_LINK;
    pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
    pdu->setMessageID(token);
    pdu->setURI((char*)uristring, strlen(uristring));

    send(pdu, token);
}

/* Virtual functions */

/* Called if a node is not answering a request */
void node::nodeNotResponding(uint16_t token){
    qDebug() << "Node: " << getAddressStr() << " token: " << token;
    token = 0;
}

/* Parse the list of published sensors from this node */
QVariant node::parseAppLinkFormat(uint16_t token, QByteArray payload){
    Q_UNUSED(token);
    qDebug() << "node: parseAppLinkFormat";

    QString pl = QString(payload);
    //All resources are separeted by a ','
    QStringList rlist = pl.split(',', QString::SkipEmptyParts);
    for(int i=0; i<rlist.count(); i++){
        QStringList slist = rlist.at(i).split(';', QString::SkipEmptyParts);
        QString uri;
        QVariantMap attributes;

        for(int j=0; j<slist.count(); j++){
            if(j==0){   //Uri
                //For now index 0 is always the uri
                uri = slist.at(0);
                uri.remove(QRegExp("[<>]"));
                if( uri.at(0) == '/' ) uri.remove( 0, 1 );    //Remove leading "/"
                //slist.removeAt(0);
            }
            else{   //Attributes as key value pairs
                QStringList keyval = slist.at(j).split("=");
                if(keyval.size() == 2){
                    attributes[keyval[0]] = keyval[1];
                }
            }
        }
        addSensor(uri, attributes);
    }
    this->token = 0;
    return QVariant(0);
}

void node::addSensor(QString uri, QVariantMap attributes){
    if(uri.compare("su/nodeinfo") == 0){
        //This is a special device, we use to handle special commands, like
        //Format the filesystem, factory reset, hw/sw version request. etc
        nodesetup = new nodeinfo(this, uri, attributes);
    }
    else if(uri.compare(".well-known/core") != 0){
        sensor* s;
        if(uri.compare("su/pulsecounter") == 0){
            s = new pulsecounter(this, uri, attributes, allsensorslist);
        }
        else if(
                uri.compare("su/powerrelay") == 0 ||
                uri.compare("su/ledindicator") == 0 ||
                uri.compare("su/led_yellow") == 0 ||
                uri.compare("su/led_red") == 0 ||
                uri.compare("su/led_orange") == 0 ||
                uri.compare("su/led_green") == 0
        ){
            s = new defaultdevice(this, uri, attributes, allsensorslist);
        }
        else{
            s = new sensor(this, uri, attributes, allsensorslist);
        }

        sensors.append(s);
        ownsensorslist->append(s);
        allsensorslist->append(s);
        emit sensorFound(uri, s->getClassType());
    }
}

QVariant node::parseAppOctetFormat(uint16_t token, QByteArray payload){
    Q_UNUSED(token);
    Q_UNUSED(payload);
    qDebug() << "node: parseAppOctetFormat";
    return QVariant(0);
}


nodeinfo::nodeinfo(node *parent, QString uri, QVariantMap attributes) : suinterface(parent->getAddress()){

    qDebug() << "Nodeinfo: " << uri << " with attribute: " << attributes << " created";

    this->parent = parent;
    this->uri = uri;
}

uint16_t nodeinfo::request_cfs_format(){

    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"cfsformat");

    return put_request(pdu, format_filesystem);
}

void nodeinfo::handleReturnCode(uint16_t token, CoapPDU::Code code){
    qDebug() << "Got token again";
}

