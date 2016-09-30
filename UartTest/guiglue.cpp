#include "guiglue.h"
#include <QDebug>

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
    di.conf.flags = METHOD_GET | METHOD_POST;
    di.conf.max_pollinterval = 2000;
    di.conf.version = 0001;
    di.lastval.type = CMP_TYPE_POSITIVE_FIXNUM;
    di.lastval.as.u8 = 0;
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
    di2.conf.flags = METHOD_GET | METHOD_PUT;
    di2.conf.max_pollinterval = -1;
    di2.conf.version = 0001;
    di2.lastval.type = CMP_TYPE_POSITIVE_FIXNUM;
    di2.lastval.as.u8 = 0;
    devinfo.append(di2);

    connect(interface, SIGNAL(reqResourceCount(rx_msg*)), this, SLOT(reqResourceCount(rx_msg*)));
    connect(interface, SIGNAL(reqConfig(rx_msg*)), this, SLOT(reqConfig(rx_msg*)));
    connect(interface, SIGNAL(reqValueUpdate(rx_msg*)), this, SLOT(reqValueUpdate(rx_msg*)));
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
        interface->frameandtx(&payloadbuf[0], len, rx_req->seqno);
    }
    else
        printf("Wrong Resource ID (resource_config)");
    delete (uint8_t*)rx_req->payload;
}

void guiglue::reqValueUpdate(rx_msg* rx_req){
    int id = *((char*)rx_req->payload);
    if(id < devinfo.count()){
        interface->frameandtx(id, &devinfo[id].lastval, rx_req->seqno);
    }
    else
        printf("Wrong Resource ID (resource_config)");
    delete rx_req;
}

/* Called from the gui */
void guiglue::updateValue(QVariant id){

    int index = id.toInt();
    devinfo[index].lastval.as.u8 = devinfo[index].lastval.as.u8 == 1 ? 0 : 1;
    interface->frameandtx(index, &devinfo[index].lastval);
    qDebug() << "ID " << index << " set to " <<  devinfo[index].lastval.as.u8;
}

#include <QDateTime>
void guiglue::updateTimerValue(){
    devinfo[0].lastval.type = CMP_TYPE_UINT64;
    devinfo[0].lastval.as.u64 = QDateTime::currentMSecsSinceEpoch();
}

