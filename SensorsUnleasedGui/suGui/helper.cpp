#include "helper.h"
#include <string.h>

namespace helper {

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
}
