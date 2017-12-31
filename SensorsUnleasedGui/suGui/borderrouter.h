#ifndef BORDERROUTER_H
#define BORDERROUTER_H
#include <QObject>
#include "node.h"
#include "nodestore.h"
#include "cmp/cmp.h"

class borderrouter : public suinterface
{
    Q_OBJECT
public:

    borderrouter(QHostAddress addr, nodestore *allnodeslist);

    Q_INVOKABLE void getNodeslist();
    void obsNodeslistChange();
    void obsDetectNode(QString query);

    QVariant parseAppOctetFormat(msgid token, QByteArray payload, CoapPDU::Code code);
    void handleReturnCode(msgid token, CoapPDU::Code code);

private:
    QString IP;
    QString uri = "su/rootNodes";
    QString uri_obs_change = "/change";
    uint16_t changetoken;

    nodestore* nodeslist;

    void parseNodeinList(cmp_ctx_t* cmp, cmp_object_t obj);

signals:
};
#endif // BORDERROUTER_H
