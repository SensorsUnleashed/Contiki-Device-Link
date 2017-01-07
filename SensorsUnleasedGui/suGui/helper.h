#ifndef HELPER_H
#define HELPER_H
#include <stdint.h>



namespace helper {
typedef union uip_ip6addr_t {
    uint8_t  u8[16];                      /* Initializer, must come first. */
    uint16_t u16[8];
} uip_ip6addr_t;

    int uiplib_ip6addrconv(const char *addrstr, uip_ip6addr_t *ipaddr);
}

#endif // HELPER_H
