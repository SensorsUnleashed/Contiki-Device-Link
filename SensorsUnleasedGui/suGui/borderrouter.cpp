#include "borderrouter.h"

borderrouter::borderrouter(QHostAddress addr, nodestore* allnodeslist) : suinterface(addr)
{
    nodeslist = allnodeslist;

//    getNodeslist();
//    obsNodeslistChange();
    obsDetectNode("fd81:3daa:fb4a:f7ae:212:4b00:41e:ac80");
}

void borderrouter::getNodeslist(){

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

void borderrouter::obsDetectNode(QString query){
    uint8_t id = 0;
    const char* querystring = query.toLatin1().data();

    const char* uristring = "su/detect";
    CoapPDU *pdu = new CoapPDU();
    pdu->setURI((char*)uristring, strlen(uristring));
    pdu->addOption(CoapPDU::COAP_OPTION_OBSERVE, 1, &id);
    pdu->addURIQuery((char*)"fd81:3daa:fb4a:f7ae:212:4b00:41e:ac80");

    changetoken = get_request(pdu, req_obs_detect);
}

#define NODE_INFO_HAS_ROUTE 1
#define NODE_INFO_UPSTREAM_VALID 2
#define NODE_INFO_DOWNSTREAM_VALID 4
#define NODE_INFO_PARENT_VALID 8
#define NODE_INFO_REJECTED 0x10

void borderrouter::parseNodeinList(cmp_ctx_t* cmp, cmp_object_t obj){
    QByteArray ipaddr;
    uint32_t lastseen = 0;
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
        lastseen = obj.as.u32;
    }

    cmp_read_object(cmp, &obj);
    if(obj.type == CMP_TYPE_UINT32){
        qDebug() << "node status = " << obj.as.u32;

        QVariantMap info;
        QHostAddress t((const uint8_t*)ipaddr.constData());
        info["address"] = t.toString();
        info["status"] = obj.as.u32;
        info["LastSeen"] = lastseen;

        //This info is not yet delivered from the border router, so lets just put in empty strings
        info["name"] = "";
        info["location"] = "";
        qDebug() << "Borderrouter reported: " << t.toString();
        nodeslist->updateWith(t, info);
    }
}

QVariant borderrouter::parseAppOctetFormat(msgid token, QByteArray payload, CoapPDU::Code code) {
    qDebug() << uri << " got message!";
    int cont = 0;
    cmp_ctx_t cmp;
    cmp_init(&cmp, payload.data(), buf_reader, 0);
    do{
        cmp_object_t obj;
        if(!cmp_read_object(&cmp, &obj)) return QVariant(0);
        QVariantMap result = cmpobjectToVariant(obj, &cmp).toMap();

            switch(token.req){
            case req_nodeslist:
                qDebug() << "req_obs_nodeslist_change";
            case req_obs_nodeslist_change:
                disableTokenRemoval(changetoken);
                qDebug() << "req_nodeslist";
                if(obj.type >= CMP_TYPE_ARRAY16 && obj.type <= CMP_TYPE_ARRAY32){
                    parseNodeinList(&cmp, obj);
                }
                break;
            case  req_obs_detect:
                qDebug() << "req_obs_detect";
                break;
            }
    }while(cmp.buf < payload.data() + payload.length());

    return QVariant(0);
}

void borderrouter::handleReturnCode(msgid token, CoapPDU::Code code) {
    qDebug() << code;

    if(token.req == req_obs_detect){
        disableTokenRemoval(changetoken);
        qDebug() << "handleReturnCode req_obs_detect";
    }
    else if(token.number == changetoken && code == CoapPDU::COAP_CONTENT){
        disableTokenRemoval(changetoken);
    }
}
