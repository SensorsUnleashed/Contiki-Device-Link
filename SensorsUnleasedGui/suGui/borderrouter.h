#ifndef BORDERROUTER_H
#define BORDERROUTER_H
#include <QObject>
#include "node.h"
#include "cmp/cmp.h"

class borderrouter : public suinterface
{
    Q_OBJECT
public:

    borderrouter(QHostAddress addr);

    Q_INVOKABLE void getNodeslist();
    void obsNodeslistChange();

    QVariant parseAppOctetFormat(uint16_t token, QByteArray payload, CoapPDU::Code code);
    void handleReturnCode(uint16_t token, CoapPDU::Code code);

private:
    QString IP;
    QString uri = "su/rootNodes";
    QString uri_obs_change = "/change";
    uint16_t changetoken;

    void parseNodeinList(cmp_ctx_t* cmp, cmp_object_t obj);

signals:
    void nodefound(QVariant info);
};
#endif // BORDERROUTER_H
