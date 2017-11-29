#include "borderrouter.h"

borderrouter::borderrouter(QHostAddress addr) : suinterface(addr)
{

}

void borderrouter::getNodeslist(){

    obsNodeslistChange();
    return;

    const char* uristring = uri.toLatin1().data();
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addURIQuery((char*)"nodes");
    get_request(pdu, req_nodeslist);
}

void borderrouter::obsNodeslistChange(){
    uint8_t id = 0;
    const char* uristring = "su/rootNodes/change";
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addOption(CoapPDU::COAP_OPTION_OBSERVE, 1, &id);
    changetoken = get_request(pdu, req_obs_nodeslist_change);
}

#define NODE_INFO_HAS_ROUTE 1
#define NODE_INFO_UPSTREAM_VALID 2
#define NODE_INFO_DOWNSTREAM_VALID 4
#define NODE_INFO_PARENT_VALID 8
#define NODE_INFO_REJECTED 0x10

void borderrouter::parseNodeinList(cmp_ctx_t* cmp, cmp_object_t obj){
    QByteArray ipaddr;

    for(uint32_t i=0; i<obj.as.array_size; i++){
        //fixme: Only handles byte arrays
        cmp_object_t obj;
        cmp_read_object(cmp, &obj);
        if(obj.type == CMP_TYPE_UINT8){
            ipaddr.append(obj.as.u8);
        }
    }
    cmp_read_object(cmp, &obj);
    if(obj.type == CMP_TYPE_UINT32){
        qDebug() << "node status = " << obj.as.u32;

        QVariantMap info;
        QHostAddress t((const uint8_t*)ipaddr.constData());
        info["address"] = t.toString();
        info["status"] = obj.as.u32;
        qDebug() << "Borderrouter reported: " << t.toString();
        emit nodefound(info);
    }
}

QVariant borderrouter::parseAppOctetFormat(uint16_t token, QByteArray payload, CoapPDU::Code code) {
    qDebug() << uri << " got message!";
    int cont = 0;
    cmp_ctx_t cmp;
    cmp_init(&cmp, payload.data(), buf_reader, 0);
    int index = findToken(token, this->token);
    int keep = 0;
    do{
        cmp_object_t obj;
        if(!cmp_read_object(&cmp, &obj)) return QVariant(0);
        QVariantMap result = cmpobjectToVariant(obj).toMap();

        //qDebug() << result;

        if(index != -1){
            switch(this->token.at(index).req){
            case req_nodeslist:
                qDebug() << "req_obs_nodeslist_change";
            case req_obs_nodeslist_change:
                keep = 1;
                disableTokenRemoval(changetoken);
                qDebug() << "req_nodeslist";
                if(obj.type >= CMP_TYPE_ARRAY16 && obj.type <= CMP_TYPE_ARRAY32){
                    parseNodeinList(&cmp, obj);
                }
                break;
            }


        }
    }while(cmp.buf < payload.data() + payload.length() && cont);

    if(keep == 0) this->token.remove(index);
    return QVariant(0);
}

void borderrouter::handleReturnCode(uint16_t token, CoapPDU::Code code) {
    qDebug() << code;

    if(token == changetoken && code == CoapPDU::COAP_CONTENT){
        disableTokenRemoval(changetoken);
    }
}
