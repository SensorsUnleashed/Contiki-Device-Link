#ifndef SOCKET_H
#define SOCKET_H

#include <QUdpSocket>
#include <QObject>
#include <QVector>

#include "wsn.h"

struct observer{
    QHostAddress id;
    wsn* ref;
};

class socket : public QObject
{
    Q_OBJECT
public:

    explicit socket(QObject *parent = 0);
    ~socket();

    static socket* getInstance();
    void observe(wsn *ref, QHostAddress id);
    void observe_stop(wsn* ref);

    void send(QHostAddress addr, uint8_t* pduptr, int len);

private:
    static bool instanceFlag;
    static socket *conn;

    QUdpSocket* udpSocket;
    QVector<struct observer> observerlist;

signals:
    void messageReceived(QHostAddress sender, QByteArray* message);

private slots:
    void readPendingDatagrams();

public slots:
};

#endif // SOCKET_H
