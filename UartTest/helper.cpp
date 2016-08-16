#include "helper.h"

namespace helper {

enum host_commands_e SToE(QString cmd){
    if(cmd.compare("HOST_CMD_GET_READINGS_PHASE_1") == 0){
        return HOST_CMD_GET_READINGS_PHASE_1;
    }


    else{
        return HOST_CMD_UNKNOWN;
    }
}

QString EToS(enum host_commands_e cmd){

}


int32_t arrayToInt(char *data, int len){

    int32_t ret = 0;
    for(int i=0; i<len-1; i++){
        if(*(data+i) < 0){
            *(data+i) *= -1;
        }
        ret += (u_int32_t)((*(data+i))) << i*8;
    }
    ret += (int32_t)((*(data+len-1))) << (len-1)*8;
    return ret;
}
}


