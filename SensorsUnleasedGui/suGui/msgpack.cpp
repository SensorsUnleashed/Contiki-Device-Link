#include "msgpack.h"


static bool buf_reader(cmp_ctx_t *ctx, void *data, uint32_t limit);
static uint32_t buf_writer(cmp_ctx_t* ctx, const void *data, uint32_t count);

msgunpack::msgunpack(QByteArray buffer){
    cmp_init(&cmp, buffer.data(), buf_reader, 0);
}

//Return 0 for ok
//Else error
int msgunpack::getResult(cmp_object_t* obj){
    if(cmp_read_object(&cmp, obj)){
        return 0;
    }
    return 1;
}

msgpack::msgpack(QByteArray buffer)
{
    //    cmp_ctx_t cmp;
    //    cmp_init(&cmp, payload.data(), buf_reader, buf_writer);

    //    cmp_write_array(&cmp, sizeof(pairaddr));
    //    for(int i=0; i<8; i++){
    //        cmp_write_u16(&cmp, pairaddr.u16[i]);
    //    }

    //    cmp_write_str(&cmp, pairurlstr.data(), pairurlstr.length());
}

QVariant msgpack::getFirst(){

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
