#include "proto1.h"
#include <QDebug>

extern "C" {
#include "crc16.h"
}

proto1::proto1(uart* comm)
{
    this->comm = comm;
    resources.append("/button/actuator");
    resources.append("/timer/counter");

    connect(comm, SIGNAL(messageReceived()), this, SLOT(handleIncomming()));
}

void proto1::handleIncomming(){
    while(!comm->messageQueEmpty()){
        QByteArray* msg = comm->getFirstMessage();

        int endindex = msg->length()-1;
        int len = msg->at(endindex-2); //Get the message len - its the 3 byte from the end
        int startindex = endindex - len - 2;

        if((startindex) < 0){   //Bad message - start was before array start
            delete msg;
            return;
        }

        //Remove bad trailing bytes
        msg->remove(0, startindex);

        //The array should now contain exactly 1 message - next parse it
        int ret = parseMessage(*msg);
        if( ret < 0)
            qDebug() << "Message failed with " << ret;

        delete msg;
    }

}

/* Returns -1 if CRC failed */
int proto1::parseMessage(QByteArray& msg){

    int crc = crc16_data((const unsigned char*)msg.data(), msg.length() - 2, 0);
    int crct = (unsigned short&) *(msg.data() + msg.length() - 2);
    qDebug() << "RX CRC calculated: " << crc;
    qDebug() << "RX CRC received  : " << crct;

    if(crc != (unsigned short&) *(msg.data() + msg.length() - 2)) return -1;

    QByteArray reply;

    uint8_t* data = (uint8_t*) msg.data();

    reply.append(*data++);  //We reply with the same seq no
    enum commands cmd = (enum commands)*data++;
    reply.append(cmd);      //We replay with the same cmd

    //Integrety of the message seems to be fine. Continue parsing
    switch(cmd){
    case resource_count:
        reply.append(resources.count());
        break;
    case resource_url:
        reply.append(resources.at(*data));
        break;
    }

    reply.append(reply.length());   //Add the message length
    crc = crc16_data((const uint8_t*)reply.data(), reply.length(), 0);

    reply.append(crc & 0xff);  //And the CRC16
    reply.append(crc >> 8);  //And the CRC16

    qDebug() << "TX CRC calculated: " << crc;

    comm->transmit(reply);  //Only thing left is to ship it

    return 1;
}
