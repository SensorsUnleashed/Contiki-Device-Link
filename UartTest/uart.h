#ifndef UART_H
#define UART_H

#include <QObject>
#include <QSerialPort>
#include <QVariant>
#include <QVector>

#define RX_BUFLEN	150
typedef struct {
    char buffer[RX_BUFLEN];
    char* wrt_ptr;
}buffer_t;

class uart : public QObject
{
    Q_OBJECT
public:
    uart();
    void transmit(const char *txbuf, uint8_t len);
    int messageQueEmpty();
    QByteArray getFirstMessage();

    void test();

private:

    //The rx_buf is the raw uart buffer - implemented as a circular buffer.
    #define RX_BUFFERS	5
    buffer_t rxbuf[RX_BUFFERS];	//Allocate 5 buffers for receiving.
    int activebuffer = 0;
    char last_c;

    QSerialPort* uartport;
    bool portfound;
    QVector<QByteArray> messages;
    void switchbuffer();
    void byteStuff(QByteArray* msg);
    void byteunStuff(QByteArray* msg);

private slots:
    void readData();

public slots:
    void setup(QString command);

signals:
    void messageReceived(buffer_t *rxbuf);
};

#endif // UART_H
