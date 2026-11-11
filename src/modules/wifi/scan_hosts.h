#ifndef __SCAN_HOSTS_H__
#define __SCAN_HOSTS_H__

#include "core/net_utils.h"
#include <FS.h>
#include <WiFi.h>
#include <stdio.h>
#include <string.h>

#include "lwip/etharp.h"
// sets number of maximum of pending requests to table size
#define ARP_MAXPENDING ARP_TABLE_SIZE

#ifndef MAC
#define MAC(addr) String(addr[0], HEX) + ":" + String(addr[1], HEX) + ":" + String(addr[2], HEX) + ":" + String(addr[3], HEX) + ":" + String(addr[4], HEX) + ":" + String(addr[5], HEX)
#endif

namespace ScanHosts {
    struct Host {
        Host(ip4_addr_t *ip, eth_addr *eth) : ip(ip->addr), mac(MAC(eth->addr)) {}
        IPAddress ip;
        String mac;
    };
}

#endif
