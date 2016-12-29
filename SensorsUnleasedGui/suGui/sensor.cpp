#include "node.h"
#include "socket.h"
QVariant cmpobjectToVariant(cmp_object_t obj);
int encode(char* buffer, cmp_object_t objTemplate, QVariant value);
static bool buf_reader(cmp_ctx_t *ctx, void *data, uint32_t limit);
static uint32_t buf_writer(cmp_ctx_t* ctx, const void *data, uint32_t count);

sensor::sensor(node* parent, QString uri, QVariantMap attributes) : wsn(parent->getAddress())
{
    qDebug() << "Sensor: " << uri << " with attribute: " << attributes << " created";
    this->uri = uri;
    ip = parent->getAddress();

    eventsActive.as.u8 = 0;
    eventsActive.type = CMP_TYPE_UINT8;
    LastValue.as.s8 = 0;
    LastValue.type = CMP_TYPE_SINT8;
    AboveEventAt.as.s8 = 0;
    AboveEventAt.type = CMP_TYPE_SINT8;
    BelowEventAt.as.s8 = 0;
    BelowEventAt.type = CMP_TYPE_SINT8;
    ChangeEvent.as.s8 = 0;
    ChangeEvent.type = CMP_TYPE_SINT8;
    RangeMin.as.s8 = 0;
    RangeMin.type = CMP_TYPE_SINT8;
    RangeMax.as.s8 = 0;
    RangeMax.type = CMP_TYPE_SINT8;

    init = 0;
}

void sensor::initSensor(){
    if(!init){
        requestRangeMin();
        requestRangeMax();
        req_eventSetup();
        init = 1;
    }
}

QVariant sensor::getConfigValues(){
    QVariantList list;

    QVariantMap result;
    result = cmpobjectToVariant(LastValue).toMap();
    result["id"] = "LastValue";
    list.append(result);
    result = cmpobjectToVariant(AboveEventAt).toMap();
    result["id"] = "AboveEventAt";
    list.append(result);
    result = cmpobjectToVariant(BelowEventAt).toMap();
    result["id"] = "BelowEventAt";
    list.append(result);
    result = cmpobjectToVariant(ChangeEvent).toMap();
    result["id"] = "ChangeEvent";
    list.append(result);
    result = cmpobjectToVariant(RangeMin).toMap();
    result["id"] = "RangeMin";
    list.append(result);
    result = cmpobjectToVariant(RangeMax).toMap();
    result["id"] = "RangeMax";
    list.append(result);
    result = cmpobjectToVariant(eventsActive).toMap();
    result["id"] = "eventsActive";
    list.append(result);

    return list;
}

//Return index of the token, -1 if not found
int findToken(uint16_t token, QVector<msgid> tokenlist){
    for(int i=0; i<tokenlist.count(); i++){
        if(tokenlist.at(i).number == token){
            return i;
        }
    }
    return -1;
}

/******** Sensor requests ***************/

void sensor::get_request(CoapPDU *pdu, enum request req, QByteArray payload){
    msgid t;
    t.req = req;
    t.number = qrand();

    pdu->setType(CoapPDU::COAP_CONFIRMABLE);
    pdu->setCode(CoapPDU::COAP_GET);
    pdu->setToken((uint8_t*)&t.number,2);

    enum CoapPDU::ContentFormat ct = CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET;
    pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
    pdu->setMessageID(t.number);

    send(pdu, t.number, payload);

    token.append(t);
}

void sensor::put_request(CoapPDU *pdu, enum request req, QByteArray payload){
    msgid t;
    t.req = req;
    t.number = qrand();

    pdu->setType(CoapPDU::COAP_CONFIRMABLE);
    pdu->setCode(CoapPDU::COAP_PUT);
    pdu->setToken((uint8_t*)&t.number,2);

    enum CoapPDU::ContentFormat ct = CoapPDU::COAP_CONTENT_FORMAT_APP_OCTET;
    pdu->addOption(CoapPDU::COAP_OPTION_CONTENT_FORMAT,1,(uint8_t*)&ct);
    pdu->setMessageID(t.number);

    send(pdu, t.number, payload);

    token.append(t);
}


void sensor::requestValue(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    get_request(pdu, req_currentValue);
}

void sensor::requestAboveEventLvl(){   
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"AboveEventAt");
    get_request(pdu, req_aboveEventValue);
}

void sensor::requestBelowEventLvl(){   
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"BelowEventAt");
    get_request(pdu, req_belowEventValue);
}

void sensor::requestChangeEventLvl(){   
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"ChangeEventAt");
    get_request(pdu, req_changeEventAt);
}

void sensor::requestRangeMin(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"RangeMin");
    get_request(pdu, req_RangeMinValue);
}

void sensor::requestRangeMax(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"RangeMax");
    get_request(pdu, req_RangeMaxValue);
}

void sensor::req_eventSetup(){
    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"getEventSetup");
    get_request(pdu, req_getEventSetup);
}

void sensor::updateConfig(QVariant updatevalues){
    qDebug() << updatevalues.toMap();
    QVariantMap values = updatevalues.toMap();
    QByteArray payload(200, 0);
    int len = 0;

    //We store the values in the same containers as with what received
    if(!values.contains("AboveEventAt")) return;
    len += encode(payload.data() + len, AboveEventAt, values["AboveEventAt"]);
    if(!values.contains("BelowEventAt")) return;
    len += encode(payload.data() + len, BelowEventAt, values["BelowEventAt"]);
    if(!values.contains("ChangeEvent")) return;
    len += encode(payload.data() + len, ChangeEvent, values["ChangeEvent"]);
    if(!values.contains("eventsActive")) return;
    len += encode(payload.data() + len, eventsActive, values["eventsActive"]);

    payload.resize(len);

    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"eventsetup");

    put_request(pdu, req_updateEventsetup, payload);
}

/******** Sensor reply handlers ************/

void sensor::nodeNotResponding(uint16_t token){
    qDebug() << "Message timed out";

    int index = findToken(token, this->token);
    if(index != -1)
        this->token.remove(index);
}

QVariant sensor::parseAppOctetFormat(uint16_t token, QByteArray payload) {
    qDebug() << uri << " got message!";

    cmp_ctx_t cmp;
    cmp_init(&cmp, payload.data(), buf_reader, 0);

    cmp_object_t obj;
    if(!cmp_read_object(&cmp, &obj)) return QVariant(0);

    QVariantMap result = cmpobjectToVariant(obj).toMap();
    qDebug() << result;

    int index = findToken(token, this->token);
    if(index != -1){
        switch(this->token.at(index).req){
        case req_RangeMinValue:
            RangeMin = obj;
            emit rangeMinValueReceived(result);
            break;
        case req_RangeMaxValue:
            RangeMax = obj;
            emit rangeMaxValueReceived(result);
            break;
        case req_currentValue:
            LastValue = obj;
            emit currentValueChanged(result);
            break;
        case req_aboveEventValue:
            AboveEventAt = obj;
            emit aboveEventValueChanged(result);
            break;
        case req_belowEventValue:
            BelowEventAt = obj;
            emit belowEventValueChanged(result);
            break;
        case req_changeEventAt:
            ChangeEvent = obj;
            emit changeEventValueChanged(result);
            break;
        case req_getEventSetup:
            AboveEventAt = obj;
            if(!cmp_read_object(&cmp, &obj)) return QVariant(0);
            BelowEventAt = obj;
            if(!cmp_read_object(&cmp, &obj)) return QVariant(0);
            ChangeEvent = obj;
            if(!cmp_read_object(&cmp, &obj)) return QVariant(0);
            eventsActive = obj;
            emit eventSetupRdy();
            break;
        case req_updateEventsetup:
            qDebug() << "req_updateEventsetup";
            qDebug() << payload;
            break;
        }
    }

    this->token.remove(index);
    return QVariant(0);
}



/*************** Helpers ******************************************/

QVariant cmpobjectToVariant(cmp_object_t obj){
    QVariantMap result;
    result["enum_no"] = obj.type;

    switch(obj.type){
    case CMP_TYPE_POSITIVE_FIXNUM:
        result["enum_str"] = "CMP_TYPE_POSITIVE_FIXNUM";
        result["value"] = obj.as.u8;
        break;
    case CMP_TYPE_NIL:
        result["enum_str"] = "CMP_TYPE_NIL";
        result["value"] = 0;
        break;
    case CMP_TYPE_UINT8:
        result["enum_str"] = "CMP_TYPE_UINT8";
        result["value"] = obj.as.u8;
        break;
    case CMP_TYPE_BOOLEAN:
        result["enum_str"] = "CMP_TYPE_BOOLEAN";
        break;
    case CMP_TYPE_FLOAT:
        result["enum_str"] = "CMP_TYPE_FLOAT";
        result["value"] = obj.as.flt;
        break;
    case CMP_TYPE_DOUBLE:
        result["enum_str"] = "CMP_TYPE_DOUBLE";
        result["value"] = obj.as.dbl;
        break;
    case CMP_TYPE_UINT16:
        result["enum_str"] = "CMP_TYPE_UINT16";
        result["value"] = obj.as.u16;
        break;
    case CMP_TYPE_UINT32:
        result["enum_str"] = "CMP_TYPE_UINT32";
        result["value"] = obj.as.u32;
        break;
    case CMP_TYPE_UINT64:
        result["enum_str"] = "CMP_TYPE_UINT64";
        //result["value"] = obj.as.u64;
        break;
    case CMP_TYPE_SINT8:
        result["enum_str"] = "CMP_TYPE_SINT8";
        result["value"] = obj.as.s8;
        break;
    case CMP_TYPE_NEGATIVE_FIXNUM:
        result["enum_str"] = "CMP_TYPE_NEGATIVE_FIXNUM";
        result["value"] = obj.as.s8;
        break;
    case CMP_TYPE_SINT16:
        result["enum_str"] = "CMP_TYPE_SINT16";
        result["value"] = obj.as.s16;
        break;
    case CMP_TYPE_SINT32:
        result["enum_str"] = "CMP_TYPE_SINT32";
        result["value"] = obj.as.s32;
        break;
    case CMP_TYPE_SINT64:
        result["enum_str"] = "CMP_TYPE_SINT64";
        //result["value"] = Q_INT64_C(obj.as.s64);
        break;
    case CMP_TYPE_FIXMAP:
        result["enum_str"] = "CMP_TYPE_FIXMAP";
        break;
    case CMP_TYPE_FIXARRAY:
        result["enum_str"] = "CMP_TYPE_FIXARRAY";
        break;
    case CMP_TYPE_FIXSTR:
        result["enum_str"] = "CMP_TYPE_FIXSTR";
        break;
    case CMP_TYPE_BIN8:
        result["enum_str"] = "CMP_TYPE_BIN8";
        break;
    case CMP_TYPE_BIN16:
        result["enum_str"] = "CMP_TYPE_BIN16";
        break;
    case CMP_TYPE_BIN32:
        result["enum_str"] = "CMP_TYPE_BIN32";
        break;
    case CMP_TYPE_EXT8:
        result["enum_str"] = "CMP_TYPE_EXT8";
        break;
    case CMP_TYPE_EXT16:
        result["enum_str"] = "CMP_TYPE_EXT16";
        break;
    case CMP_TYPE_EXT32:
        result["enum_str"] = "CMP_TYPE_EXT32";
        break;
    case CMP_TYPE_FIXEXT1:
        result["enum_str"] = "CMP_TYPE_FIXEXT1";
        break;
    case CMP_TYPE_FIXEXT2:
        result["enum_str"] = "CMP_TYPE_FIXEXT2";
        break;
    case CMP_TYPE_FIXEXT4:
        result["enum_str"] = "CMP_TYPE_FIXEXT4";
        break;
    case CMP_TYPE_FIXEXT8:
        result["enum_str"] = "CMP_TYPE_FIXEXT8";
        break;
    case CMP_TYPE_FIXEXT16:
        result["enum_str"] = "CMP_TYPE_FIXEXT16";
        break;
    case CMP_TYPE_STR8:
        result["enum_str"] = "CMP_TYPE_STR8";
        break;
    case CMP_TYPE_STR16:
        result["enum_str"] = "CMP_TYPE_STR16";
        break;
    case CMP_TYPE_STR32:
        result["enum_str"] = "CMP_TYPE_STR32";
        break;
    case CMP_TYPE_ARRAY16:
        result["enum_str"] = "CMP_TYPE_ARRAY16";
        break;
    case CMP_TYPE_ARRAY32:
        result["enum_str"] = "CMP_TYPE_ARRAY32";
        break;
    case CMP_TYPE_MAP16:
        result["enum_str"] = "CMP_TYPE_MAP16";
        break;
    case CMP_TYPE_MAP32:
        result["enum_str"] = "CMP_TYPE_MAP32";
        break;
    }
    return result;
}

cmp_object_t QVariantToCmpobject(uint8_t type, QVariant value){

    cmp_object_t obj;
    obj.type = type;

    switch(type){
    case CMP_TYPE_POSITIVE_FIXNUM:
    case CMP_TYPE_NIL:
    case CMP_TYPE_UINT8:
        obj.as.u8 = value.toUInt();
        break;
    case CMP_TYPE_BOOLEAN:
        obj.as.boolean = value.toBool();
        break;
    case CMP_TYPE_FLOAT:
        obj.as.flt = value.toFloat();
        break;
    case CMP_TYPE_DOUBLE:
        obj.as.dbl = value.toDouble();
        break;
    case CMP_TYPE_UINT16:
        obj.as.u16 = value.toUInt();
        break;
    case CMP_TYPE_UINT32:
        obj.as.u32 = value.toUInt();
        break;
    case CMP_TYPE_UINT64:
        obj.as.u64 = value.toUInt();
        break;
    case CMP_TYPE_SINT8:
    case CMP_TYPE_NEGATIVE_FIXNUM:
        obj.as.s8 = value.toInt();
        break;
    case CMP_TYPE_SINT16:
        obj.as.s16 = value.toInt();
        break;
    case CMP_TYPE_SINT32:
        obj.as.s32 = value.toInt();
        break;
    case CMP_TYPE_SINT64:
        obj.as.s64 = value.toInt();
        break;
    case CMP_TYPE_FIXMAP:
    case CMP_TYPE_FIXARRAY:
    case CMP_TYPE_FIXSTR:
    case CMP_TYPE_BIN8:
    case CMP_TYPE_BIN16:
    case CMP_TYPE_BIN32:
    case CMP_TYPE_EXT8:
    case CMP_TYPE_EXT16:
    case CMP_TYPE_EXT32:
    case CMP_TYPE_FIXEXT1:
    case CMP_TYPE_FIXEXT2:
    case CMP_TYPE_FIXEXT4:
    case CMP_TYPE_FIXEXT8:
    case CMP_TYPE_FIXEXT16:
    case CMP_TYPE_STR8:
    case CMP_TYPE_STR16:
    case CMP_TYPE_STR32:
    case CMP_TYPE_ARRAY16:
    case CMP_TYPE_ARRAY32:
    case CMP_TYPE_MAP16:
    case CMP_TYPE_MAP32:
        qDebug() << "QVariantToCmpobject: Type not yet implemented";
        break;
    }
    return obj;
}

/* Returns the len of the encoded message */
int encode(char* buffer, cmp_object_t objTemplate, QVariant value){
    cmp_ctx_t cmp;
    cmp_init(&cmp, buffer, 0, buf_writer);

    cmp_object_t obj = QVariantToCmpobject(objTemplate.type, value);

    cmp_write_object(&cmp, &obj);
    return (char*)cmp.buf - buffer;
}

static bool buf_reader(cmp_ctx_t *ctx, void *data, uint32_t limit) {

    uint8_t* dataptr = (uint8_t*)data;
    uint8_t* bufptr = (uint8_t*)ctx->buf;

    for(uint32_t i=0; i<limit; i++){
        *dataptr++ = *bufptr++;
    }

    data = dataptr;
    ctx->buf = bufptr;

    return true;
}

static uint32_t buf_writer(cmp_ctx_t* ctx, const void *data, uint32_t count){

    uint8_t* dataptr = (uint8_t*)data;
    uint8_t* bufptr = (uint8_t*)ctx->buf;

    for(uint32_t i=0; i<count; i++){
        *bufptr++ = *dataptr++;
    }
    data = dataptr;
    ctx->buf = bufptr;

    return count;
}
