#include "uart.h"
#include <QDebug>
#include <QThread>

#define END     (const char)0xC0
#define ESC     (const char)0xDB
#define ESC_END (const char)0xDC
#define ESC_ESC (const char)0xDD

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
    uartport = new QSerialPort("/dev/ttyUSB0");
    //uartport = new QSerialPort("/dev/pts/0");

    activebuffer = 0;
    switchbuffer();

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

uart::~uart(){
    uartport->close();
    delete uartport;
}

void uart::switchbuffer(){
    activebuffer++;
    activebuffer = activebuffer >= RX_BUFFERS ? 0 : activebuffer;

    //Reset the write pointer
    rxbuf[activebuffer].wrt_ptr = &rxbuf[activebuffer].buffer[0];
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

/* Read into a circular buffer and unstuff the messages
 * I didn't have to be a circular buffer, but for debugging
 * it was nice to have the possibility to see back in time.
*/
void uart::readData(){

    char c;

    while(!uartport->atEnd()){
        uartport->read(&c, 1);

        if(c == END){	//Is it the last byte in a frame
            emit messageReceived(&rxbuf[activebuffer]);
            switchbuffer();
        }
        else if(c == ESC_ESC && last_c == ESC){
            *rxbuf[activebuffer].wrt_ptr++ = last_c;
        }
        else if(c == ESC_END && last_c == ESC){
            *rxbuf[activebuffer].wrt_ptr++ = END;
        }
        else{
            *rxbuf[activebuffer].wrt_ptr++ = c;
        }
        last_c = c;
    }
}

void uart::transmit(const char* txbuf, uint8_t len){

    QByteArray tx(txbuf, len);
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
