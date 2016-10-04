#include "guiglue.h"
#include <QDebug>
#include <QTimer>

guiglue::guiglue(proto1 *protohandler)
{
    interface = protohandler;

    stringbuffer = new char[300];
    char* wrtptr = stringbuffer;

    struct deviceinfo_s di;
    di.conf.group = wrtptr;
    wrtptr += sprintf(wrtptr, "button") + 1;
    di.conf.type = wrtptr;
    wrtptr += sprintf(wrtptr, "actuator") + 1;
    di.conf.attr = wrtptr;
    wrtptr += sprintf(wrtptr, "title=\"Green LED\" ;rt=\"Control\"") + 1;
    di.conf.spec = wrtptr;
    wrtptr += sprintf(wrtptr, "ON=1, OFF=0") + 1;
    di.conf.unit = wrtptr;
    wrtptr += sprintf(wrtptr, " ") + 1;
    di.conf.resolution = 100;
    di.conf.hysteresis = 10;
    di.conf.flags = METHOD_GET | METHOD_POST | IS_OBSERVABLE;
    di.conf.max_pollinterval = 2000;
    di.conf.version = 0001;
    di.conf.AboveEventAt.type = CMP_TYPE_SINT8;
    di.conf.AboveEventAt.as.s8 = 1;
    di.conf.AboveEventAt.type = CMP_TYPE_SINT8;
    di.conf.BelowEventAt.as.s8 = 0;
    di.conf.eventsActive = 1;
    di.lastval.type = CMP_TYPE_POSITIVE_FIXNUM;
    di.lastval.as.u8 = 1;
    devinfo.append(di);

    struct deviceinfo_s di2;
    di2.conf.group = wrtptr;
    wrtptr += sprintf(wrtptr, "timer") + 1;
    di2.conf.type = wrtptr;
    wrtptr += sprintf(wrtptr, "counter") + 1;
    di2.conf.attr = wrtptr;
    wrtptr += sprintf(wrtptr, "title=\"Orange LED\";rt=\"Control\"") + 1;
    di2.conf.spec = wrtptr;
    wrtptr += sprintf(wrtptr, "1 sec") + 1;
    di2.conf.unit = wrtptr;
    wrtptr += sprintf(wrtptr, "Sec") + 1;
    di2.conf.resolution = 100;
    di2.conf.hysteresis = 1;
    di2.conf.flags = METHOD_GET | METHOD_PUT | IS_OBSERVABLE;
    di2.conf.max_pollinterval = -1;
    di2.conf.version = 0001;
    di2.lastval.type = CMP_TYPE_POSITIVE_FIXNUM;
    di2.lastval.as.u8 = 1;
    di2.conf.AboveEventAt.as.s64 = 0;
    di2.conf.BelowEventAt.as.s64 = 0;
    di2.conf.eventsActive = 1;
    devinfo.append(di2);

    QTimer* tick = new QTimer;
    tick->setInterval(10000);
    tick->start();
    connect(tick, SIGNAL(timeout()), this, SLOT(updateTimerValue()));
    connect(interface, SIGNAL(reqResourceCount(rx_msg*)), this, SLOT(reqResourceCount(rx_msg*)));
    connect(interface, SIGNAL(reqConfig(rx_msg*)), this, SLOT(reqConfig(rx_msg*)));
    connect(interface, SIGNAL(reqValueUpdate(rx_msg*)), this, SLOT(reqValueUpdate(rx_msg*)));
    connect(interface, SIGNAL(reqValueUpdateAll(rx_msg*)), this, SLOT(reqValueUpdateAll(rx_msg*)));
}
void guiglue::reqResourceCount(rx_msg* rx_req){
    char c = devinfo.count();
    uint8_t buf[100];
    int len = cp_encodemessage(rx_req->seqno, resource_count, &c, 1, &buf[0]);
    interface->transmit((const char*)&buf[0], len);
    delete (uint8_t*)rx_req->payload;
}

void guiglue::reqConfig(rx_msg* rx_req){
    int id = *((char*)rx_req->payload);
    if(id < devinfo.count()){
        uint8_t payloadbuf[200];
        int len = cp_encoderesource_conf(&devinfo[id].conf, &payloadbuf[0]);
        interface->frameandtx(&payloadbuf[0], len, rx_req->cmd, rx_req->seqno);
    }
    else
        printf("Wrong Resource ID (resource_config)");
    delete (uint8_t*)rx_req->payload;
}

void guiglue::reqValueUpdateAll(rx_msg* rx_req){
    for(int i=0; i<devinfo.count(); i++){
        interface->frameandtx(i, &devinfo[i].lastval, resource_value_update, rx_req->seqno);
    }
}

void guiglue::reqValueUpdate(rx_msg* rx_req){
    int id = *((char*)rx_req->payload);
    if(id < devinfo.count()){
        interface->frameandtx(id, &devinfo[id].lastval, resource_value_update, rx_req->seqno);
    }
    else
        printf("Wrong Resource ID (resource_config)");
    delete rx_req;
}

/* Called from the gui */
void guiglue::updateValue(QVariant id){

    int index = id.toInt();
    devinfo[index].lastval.as.u8 = devinfo[index].lastval.as.u8 == 1 ? 0 : 1;
    qDebug() << "ID " << index << " set to " <<  devinfo[index].lastval.as.u8;

    if(devinfo[index].conf.eventsActive == 1 && (devinfo[index].lastval.as.u8 >= devinfo[index].conf.AboveEventAt.as.s8 || devinfo[index].lastval.as.u8 <= devinfo[index].conf.BelowEventAt.as.s8)){
        interface->frameandtx(index, &devinfo[index].lastval, resource_event);
    }
    else{
        interface->frameandtx(index, &devinfo[index].lastval, resource_value_update);
    }
}

#include <QDateTime>
void guiglue::updateTimerValue(){
//    devinfo[1].lastval.type = CMP_TYPE_SINT64;
//    devinfo[1].lastval.as.s64 = QDateTime::currentMSecsSinceEpoch();

    devinfo[1].lastval.type = CMP_TYPE_UINT32;
    devinfo[1].lastval.as.u32 = (uint32_t)QDateTime::currentMSecsSinceEpoch();

    if(devinfo[1].conf.eventsActive == 1){
        interface->frameandtx(1, &devinfo[1].lastval, resource_event);
    }
    else{
        interface->frameandtx(1, &devinfo[1].lastval, resource_value_update);
    }

    qDebug() << "Tick";
}

