#ifndef UART_H
#define UART_H

#include <QObject>
#include <QSerialPort>
#include <QVariant>
#include <QVector>

class uart : public QObject
{
    Q_OBJECT
public:
    uart();
    void transmit(QByteArray data);
    int messageQueEmpty();
    QByteArray* getFirstMessage();

    void test();

private:
    QSerialPort* uartport;
    bool portfound;
    QByteArray* recbytes;
    QVector<QByteArray*> messages;

    void byteStuff(QByteArray* msg);
    void byteunStuff(QByteArray* msg);

private slots:
    void readData();

public slots:
    void setup(QString command);

signals:
    void messageReceived(void);
};

#endif // UART_H
