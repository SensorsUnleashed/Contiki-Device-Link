#include "guiglue.h"
#include <QDebug>
#include <QTimer>
#include <QDateTime>

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
    di.conf.flags = METHOD_GET | METHOD_PUT  | IS_OBSERVABLE;
    di.conf.max_pollinterval = 2000;
    di.conf.version = 0001;
    di.conf.AboveEventAt.type = CMP_TYPE_UINT8;
    di.conf.AboveEventAt.as.u8 = 10;
    di.conf.BelowEventAt.type = CMP_TYPE_UINT8;
    di.conf.BelowEventAt.as.u8 = 10;
    di.conf.ChangeEvent.type = CMP_TYPE_UINT8;
    di.conf.ChangeEvent.as.u8 = 1;
    di.conf.eventsActive = 1;
    di.lastval.type = CMP_TYPE_POSITIVE_FIXNUM;
    di.lastval.as.u8 = 1;
    devinfo.append(di);

    direction = 1;  //1=up, -1=down

    struct deviceinfo_s di2;
    di2.conf.group = wrtptr;
    wrtptr += sprintf(wrtptr, "timer") + 1;
    di2.conf.type = wrtptr;
    wrtptr += sprintf(wrtptr, "counter") + 1;
    di2.conf.attr = wrtptr;
    wrtptr += sprintf(wrtptr, "title=\"Timer\";rt=\"Control\"") + 1;
    di2.conf.spec = wrtptr;
    wrtptr += sprintf(wrtptr, "Res: 1 sec") + 1;
    di2.conf.unit = wrtptr;
    wrtptr += sprintf(wrtptr, "Sec") + 1;
    di2.conf.resolution = 100;
    di2.conf.flags = METHOD_GET | METHOD_PUT | IS_OBSERVABLE;
    di2.conf.max_pollinterval = -1;
    di2.conf.version = 0001;
    di2.lastval.type = CMP_TYPE_UINT32;
    di2.lastval.as.u32 = (uint32_t)(QDateTime::currentMSecsSinceEpoch()/1000);
    di2.conf.AboveEventAt.type = CMP_TYPE_UINT32;
    di2.conf.AboveEventAt.as.u32 = 0;
    di2.conf.BelowEventAt.type = CMP_TYPE_UINT32;
    di2.conf.BelowEventAt.as.u32 = 0;
    di2.conf.ChangeEvent.type = CMP_TYPE_UINT8;
    di2.conf.ChangeEvent.as.u8 = 10;    //event every 10 sec
    di2.conf.eventsActive = 1;
    devinfo.append(di2);

    QTimer* tick = new QTimer;
    tick->setInterval(1000);
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
    qDebug() << "reqConfig id: " << id;
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

    if(direction == 1){
        devinfo[index].lastval.as.u8 += 1;
        if(devinfo[index].lastval.as.u8 == 4) direction = -1;
    }
    else{
        devinfo[index].lastval.as.u8 -= 1;
        if(devinfo[index].lastval.as.u8 == 1) direction = 1;
    }
    qDebug() << "ID " << index << " set to " <<  devinfo[index].lastval.as.u8;
    interface->frameandtx(index, &devinfo[index].lastval, resource_event);
}

void guiglue::updateTimerValue(){
    devinfo[1].lastval.type = CMP_TYPE_UINT32;
    devinfo[1].lastval.as.u32 = (uint32_t)(QDateTime::currentMSecsSinceEpoch() / 1000); //Second tick

    if(devinfo[1].conf.eventsActive == 1 && (devinfo[1].lastval.as.u32 % devinfo[1].conf.ChangeEvent.as.u8 == 0)){
        interface->frameandtx(1, &devinfo[1].lastval, resource_event);
    }
    else{
        interface->frameandtx(1, &devinfo[1].lastval, resource_value_update);
    }

    qDebug() << "Tick";
}

