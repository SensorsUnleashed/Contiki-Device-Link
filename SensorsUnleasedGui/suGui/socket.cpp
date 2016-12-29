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
