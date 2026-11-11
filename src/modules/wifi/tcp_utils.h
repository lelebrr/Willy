#ifndef TCP_UTILS_H
#define TCP_UTILS_H

#include "core/mykeyboard.h"
#include <Arduino.h>

void listenTcpPort(int port = 0);
void clientTCP();
void listenUdpPort();
void clientUDP();

#endif // TCP_UTILS_H
