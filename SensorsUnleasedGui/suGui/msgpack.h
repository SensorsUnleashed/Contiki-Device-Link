#ifndef MSGPACK_H
#define MSGPACK_H

#include <QByteArray>
#include <QVariant>

#include "cmp/cmp.h"

class msgunpack{
public:
    msgunpack(QByteArray buffer);
    int getResult(cmp_object_t* obj);

private:
    cmp_ctx_t cmp;
};

class msgpack
{
public:
    msgpack(QByteArray buffer);

    QVariant getFirst();
};

#endif // MSGPACK_H
