#ifndef HELPER_H
#define HELPER_H

#include "dlt645protocol.h"

namespace helper {
    enum host_commands_e SToE(QString cmd);

    QString EToS(enum host_commands_e cmd);

    int32_t arrayToInt(char *data, int ret);
}

#endif // HELPER_H
