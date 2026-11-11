#if !defined(LITE_VERSION)

#ifndef ESP_NETIF_ETHERNET_CLIENT_H
#define ESP_NETIF_ETHERNET_CLIENT_H

#include "Arduino.h"

class ESPNetifEthernetClient {
private:
    int _sockfd;

public:
    ESPNetifEthernetClient();
    ~ESPNetifEthernetClient();
    int connect(IPAddress ip, uint16_t port, int32_t timeout_ms);
    void client_close(int sockfd = -1);
    bool connected(int sockfd = -1);
};

#endif
#endif
