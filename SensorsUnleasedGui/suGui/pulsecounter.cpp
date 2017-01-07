#include "node.h"

pulsecounter::pulsecounter(node* parent, QString uri, QVariantMap attributes) :
    sensor(parent, uri, attributes){
    polltimer = new QTimer;
    polltimer->setSingleShot(true);

    connect(polltimer, SIGNAL(timeout()), this, SLOT(doPoll()));
}

void pulsecounter::startPoll(QVariant interval){
    if(interval.toInt() == 0){
        polltimer->stop();
        return;
    }
    polltimer->setInterval(interval.toInt() * 1000);
    doPoll();
}

void pulsecounter::doPoll(){
    requestValue();
    polltimer->start();
}

