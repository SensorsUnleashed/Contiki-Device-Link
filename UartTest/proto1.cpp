#include "proto1.h"
#include <QDebug>

extern "C" {
#include "crc16.h"
}

proto1::proto1(uart* comm)
{
    this->comm = comm;

    QVariantMap map;
    map["resource_url"] = "button/actuator";
    map["resource_attributes"] = "title=\"Green LED\" ;rt=\"Control\"";
    map["flags"] = METHOD_GET | METHOD_POST;
    resources.append(map);

    map["resource_url"] = "timer/counter";
    map["resource_attributes"] = "title=\"Orange LED\";rt=\"Control\"";
    map["flags"] = METHOD_GET | METHOD_PUT;;
    resources.append(map);

    map["resource_url"] = "sensor/temp";
    map["resource_attributes"] = "title=\"Temperature\";rt=\"Temperature\";obs;";
    map["flags"] = METHOD_GET;
    resources.append(map);

    connect(comm, SIGNAL(messageReceived()), this, SLOT(handleIncomming()));

    //Used for testing RX
    //comm->test();
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

/*
 * Returns:
 *  -1 if CRC failed
 *  -2 if resource_url index is out of bounds
*/
int proto1::parseMessage(QByteArray& msg){

    int crc = crc16_data((const unsigned char*)msg.data(), msg.length() - 2, 0);
    if(crc != (unsigned short&) *(msg.data() + msg.length() - 2)) return -1;

    QByteArray reply;

    uint8_t* data = (uint8_t*) msg.data();

    reply.append(*data++);  //We reply with the same seq no
    enum commands cmd = (enum commands)*data++;
    reply.append(cmd);      //We reply with the same cmd

    //Integrety of the message seems to be fine. Continue parsing
    switch(cmd){
    case resource_count:
        reply.append(resources.count());
        break;
    case resource_url:
        if(*data > (resources.count()-1)){
            qDebug() << "resource_url: Requested index " << *data << " but max is " << resources.count()-1;
            return - 2;
        }
        else{
            reply.append(resources.at(*data)["resource_url"].toString());
            reply.append('\0'); //Unlinke c++, a string in c is ended with the \0
            qDebug() << "ID: " << *data << " = " << resources.at(*data)["resource_url"].toString();
        }
        break;
    case resource_attributes:
        if(*data > (resources.count()-1)){
            qDebug() << "resource_attributes: Requested index " << *data << " but max is " << resources.count()-1;
            return - 3;
        }
        else{
            reply.append(resources.at(*data)["resource_attributes"].toString());
            reply.append('\0'); //Unlinke c++, a string in c is ended with the \0
            //qDebug() << resources.at(*data)["resource_attributes"].toString();
        }
        break;
    case resource_flags:
        if(*data > (resources.count()-1)){
            qDebug() << "resource_flags: Requested index " << *data << " but max is " << resources.count()-1;
            return - 4;
        }
        else{
            reply.append(resources.at(*data)["flags"].toInt());
            reply.append('\0'); //Unlinke c++, a string in c is ended with the \0
            qDebug() << "id " << *data << resources.at(*data)["flags"].toInt();
        }
        break;
    case debugstring:
            qDebug() << "Debug: " << (char*)data;
        break;
    }

    reply.append(reply.length());   //Add the message length
    crc = crc16_data((const uint8_t*)reply.data(), reply.length(), 0);

    reply.append(crc & 0xff);  //And the CRC16
    reply.append(crc >> 8);  //And the CRC16

    comm->transmit(reply);  //Only thing left is to ship it

    return 1;
}
