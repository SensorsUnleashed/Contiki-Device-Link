#include "sensorsunleashed.h"
#include "../../apps/uartsensors/uart_protocolhandler.h"

typedef union uip_ip6addr_t {
    uint8_t  u8[16];                      /* Initializer, must come first. */
    uint16_t u16[8];
} uip_ip6addr_t;

static bool buf_reader(cmp_ctx_t *ctx, void *data, uint32_t limit);
static uint32_t buf_writer(cmp_ctx_t* ctx, const void *data, uint32_t count);
int uiplib_ip6addrconv(const char *addrstr, uip_ip6addr_t *ipaddr);

sensorsunleashed::sensorsunleashed(database *db, coaphandler *coap)
{
    Q_UNUSED(db);
    nodecomm = coap;
}

QVariant sensorsunleashed::pair(QVariant nodeaddr, QVariant uri, QVariant options, QVariant pairdata, QVariant oldtoken){

    uip_ip6addr_t pairaddr;
    QVariantMap map = pairdata.toMap();

    //Find out if all neccessary informations is available
    if(!map.contains("addr")) return QVariant(-1);
    if(!map.contains("url")) return QVariant(-1);

    QByteArray pairaddrstr = map["addr"].toString().toLatin1();
    if(!uiplib_ip6addrconv(pairaddrstr.data(), &pairaddr)) return QVariant(-1);

    QByteArray pairurlstr = map["url"].toString().toLatin1();
    if(!pairurlstr.length()) return QVariant(-1);

    QByteArray payload;
    payload.resize(200);

    cmp_ctx_t cmp;
    cmp_init(&cmp, payload.data(), buf_reader, buf_writer);

    cmp_write_array(&cmp, sizeof(pairaddr));
    for(int i=0; i<8; i++){
        cmp_write_u16(&cmp, pairaddr.u16[i]);
    }

    cmp_write_str(&cmp, pairurlstr.data(), pairurlstr.length());

    payload.resize((uint8_t*)cmp.buf - (uint8_t*)payload.data());

    return nodecomm->reqGet(nodeaddr, uri, options, oldtoken, payload);
}

QVariant sensorsunleashed::put(QVariant nodeaddr, QVariant uri, QVariant options, QVariant data, QVariant oldtoken){

    QByteArray payload;
    QVariantMap map = data.toMap();
    payload.resize(20);

    //Find out if all neccessary informations is available
    if(!map.contains("value")) return QVariant(-1);
    if(!map.contains("format")) return QVariant(-1);

    cmp_object_t obj;
    obj.type = map["format"].toUInt();

    switch(obj.type){
    case CMP_TYPE_BOOLEAN:
        obj.as.boolean = map["value"].toBool();
        break;
    case CMP_TYPE_FLOAT:
        obj.as.flt = map["value"].toFloat();
        break;
    case CMP_TYPE_DOUBLE:
        obj.as.dbl = map["value"].toDouble();
        break;
    case CMP_TYPE_UINT8:
        obj.as.u8 = (uint8_t)map["value"].toUInt();
        break;
    case CMP_TYPE_UINT16:
        obj.as.u16 = (uint16_t)map["value"].toUInt();
        break;
    case CMP_TYPE_UINT32:
        obj.as.u32 = (uint32_t)map["value"].toUInt();
        break;
    case CMP_TYPE_SINT8:
        obj.as.s8 = (int8_t)map["value"].toInt();
        break;
    case CMP_TYPE_SINT16:
        obj.as.s16 = (int16_t)map["value"].toInt();
        break;
    case CMP_TYPE_SINT32:
        obj.as.s32 = (int32_t)map["value"].toInt();
        break;
    }

    cmp_ctx_t cmp;
    cmp_init(&cmp, payload.data(), buf_reader, buf_writer);
    cmp_write_object(&cmp, &obj);
    payload.resize((uint8_t*)cmp.buf - (uint8_t*)payload.data());

    return nodecomm->reqGet(nodeaddr, uri, options, oldtoken, payload);
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


/* Borrowed from CONTIKI's uiplib */
int uiplib_ip6addrconv(const char *addrstr, uip_ip6addr_t *ipaddr)
{
    uint16_t value;
    int tmp, zero;
    unsigned int len;
    char c = 0;  //gcc warning if not initialized

    value = 0;
    zero = -1;
    if(*addrstr == '[') addrstr++;

    for(len = 0; len < sizeof(uip_ip6addr_t) - 1; addrstr++) {
        c = *addrstr;
        if(c == ':' || c == '\0' || c == ']' || c == '/') {
            ipaddr->u8[len] = (value >> 8) & 0xff;
            ipaddr->u8[len + 1] = value & 0xff;
            len += 2;
            value = 0;

            if(c == '\0' || c == ']' || c == '/') {
                break;
            }

            if(*(addrstr + 1) == ':') {
                /* Zero compression */
                if(zero < 0) {
                    zero = len;
                }
                addrstr++;
            }
        } else {
            if(c >= '0' && c <= '9') {
                tmp = c - '0';
            } else if(c >= 'a' && c <= 'f') {
                tmp = c - 'a' + 10;
            } else if(c >= 'A' && c <= 'F') {
                tmp = c - 'A' + 10;
            } else {
                return 0;
            }
            value = (value << 4) + (tmp & 0xf);
        }
    }
    if(c != '\0' && c != ']' && c != '/') {
        return 0;
    }
    if(len < sizeof(uip_ip6addr_t)) {
        if(zero < 0) {
            return 0;
        }
        memmove(&ipaddr->u8[zero + sizeof(uip_ip6addr_t) - len],
                &ipaddr->u8[zero], len - zero);
        memset(&ipaddr->u8[zero], 0, sizeof(uip_ip6addr_t) - len);
    }

    return 1;
}



