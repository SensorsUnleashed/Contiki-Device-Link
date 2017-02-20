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
#include "socket.h"

bool socket::instanceFlag = false;
socket* socket::conn = NULL;

socket::socket(QObject *parent) : QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    if(udpSocket->bind(QHostAddress::AnyIPv6, 5683)){
        qDebug() << "Successfully bound to Localhost port 5683";
    }

    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

socket::~socket()
{
    instanceFlag = false;
    delete udpSocket;
}

socket* socket::getInstance()
{
    if(! instanceFlag)
    {
        conn = new socket();
        instanceFlag = true;
        return conn;
    }
    else
    {
        return conn;
    }
}

void socket::observe(wsn* ref, QHostAddress id){
    struct observer o = { id, ref };

    //Check if observer is already present
    for(int i=0; i<observerlist.count(); i++){
        if(ref == observerlist.at(i).ref)
            return;
    }
    observerlist.append(o);
}

void socket::observe_stop(wsn* ref){

    //Check if observer is already present
    for(int i=0; i<observerlist.count(); i++){
        if(ref == observerlist.at(i).ref){
            observerlist.remove(i);
            return;
        }
    }
}

void socket::send(QHostAddress addr, uint8_t* pduptr, int len){
    udpSocket->writeDatagram((char*)pduptr, len, addr, 5683);
}

void socket::readPendingDatagrams(){
    while (udpSocket->hasPendingDatagrams()) {
        //Create the bytearray and have the one using it handle it.
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        /* Now pass this message on to the right receiver */
        for(int i=0; i<observerlist.count(); i++){
            if(observerlist.at(i).id == sender)
                observerlist.at(i).ref->parseData(datagram);
        }
    }
}
