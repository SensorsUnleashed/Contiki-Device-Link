#include "uart.h"
#include <QDebug>
#include <QThread>

#define END     (char)0xC0
#define ESC     (char)0xDB
#define ESC_END (char)0xDC
#define ESC_ESC (char)0xDD

#define SOH     (char)0x01
#define STX     (char)0x02  /* start of 1024-byte data packet */
#define EOT     (char)0x04
#define ACK     (char)0x06
#define NAK     (char)0x15
#define CAN     (char)0x18

uart::uart()
{
    uartport = 0;
    portfound = false;
    recbytes = new QByteArray();

    uartport = new QSerialPort("/dev/ttyUSB0");

    if(!uartport) return;   //If we did not create a uartport just return

    if(!uartport->open(QIODevice::ReadWrite)){
        return;
    }

    if(!uartport->setFlowControl(QSerialPort::NoFlowControl)) return;
    if(!uartport->setParity(QSerialPort::NoParity)) return;
    if(!uartport->setBaudRate(QSerialPort::Baud115200, QSerialPort::AllDirections)) return;
    if(!uartport->setDataBits(QSerialPort::Data8)) return;
    if(!uartport->setStopBits(QSerialPort::OneStop)) return;

    portfound = true;

    qDebug() << "IO board communication port setup!!";

    uartport->clear();  //Clean up, before starting to listen

    //Recive every message that the uart class finds valid
    connect(uartport, SIGNAL(readyRead()), this, SLOT(readData()));
}

void uart::setup(QString command){
    if(command.compare("DTR Toggle") == 0){

        if(uartport->isDataTerminalReady()){
            uartport->setDataTerminalReady(false);
            qDebug() << "setDataTerminalReady(false)";
        }
        else{
            uartport->setDataTerminalReady(true);
            qDebug() << "setDataTerminalReady(true)";
        }
        qDebug() << uartport->pinoutSignals();
    }
    else if(command.compare("RTS Toggle") == 0){
        if(uartport->isRequestToSend()){
            uartport->setRequestToSend(false);
            qDebug() << "setRequestToSend(false)";
        }
        else{
            uartport->setRequestToSend(true);
            qDebug() << "setRequestToSend(true)";
        }
        qDebug() << uartport->pinoutSignals();
    }
}

void uart::readData(){

    QByteArray peek = uartport->peek(500);
    QString valueInHex;
    for(int i=0; i<peek.length(); i++){

        valueInHex += QString("0x%1 ").arg((quint8)peek[i] , 0, 16);
    }
    qDebug() << "RX: " << valueInHex;

    //Handle thos later on
    if(peek.contains(ACK)) qDebug() << "ACK";
    else if(peek.contains(NAK)) qDebug() << "NAK";

    recbytes->append(uartport->readAll());

    if(recbytes->contains(END)){
        byteunStuff(recbytes);
        messages.append(recbytes);
        //Create a new recbytes buffer for the next message arriving
        recbytes = new QByteArray();
        emit messageReceived();
    }
}

int uart::messageQueEmpty(){
    return messages.isEmpty();
}

QByteArray *uart::getFirstMessage(){
    QByteArray* msg = messages.first();
    messages.removeFirst();
    return msg;
}

void uart::transmit(QByteArray tx){

    byteStuff(&tx);
    tx.append(END);
    uartport->write(tx);

    QString valueInHex;
    for(int i=0; i<tx.length(); i++){

        valueInHex += QString("0x%1 ").arg((quint8)tx[i] , 0, 16);
    }
    qDebug() << "TX: " << valueInHex;
}

/*
    SLIP encode the message
*/
void uart::byteStuff(QByteArray* msg){
    for(int i=0; i<msg->size(); i++){
        /*if the END byte occurs in the data to be sent, the two byte sequence ESC, ESC_END is sent instead*/
        if(msg->at(i) == END){
            msg->remove(i, 1);
            msg->insert(i, ESC_END);    //Insert before
            msg->insert(i, ESC);
        }
        /*if the ESC byte occurs in the data, the two byte sequence ESC, ESC_ESC is sent.*/
        else if(msg->at(i) == ESC){
            msg->insert(i+1, ESC_ESC);
        }
    }
}

/*
    SLIP decode the message
*/
void uart::byteunStuff(QByteArray* msg){

    for(int i=0; i<msg->size(); i++){
        /*if the ESC byte occurs in the data, the two byte sequence ESC, ESC_ESC is sent.*/
        if(i > 0 && msg->at(i) == ESC_ESC && msg->at(i-1) == ESC){
            msg->remove(i, 1);  //Remove it.
        }
        /*if the END byte occurs in the data to be sent, the two byte sequence ESC, ESC_END is sent instead */
        else if(i > 0 && msg->at(i) == ESC_END && msg->at(i-1) == ESC){
            msg->remove(i-1, 2);  //Remove it.
            msg->insert(i-1, END);
        }
        /* If the message contains a end token , remove it. */
        else if(msg->at(i) == END) {
            msg->remove(i, 1);
        }
    }
}
