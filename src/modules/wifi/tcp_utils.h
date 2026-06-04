#ifndef TCP_UTILS_H
#define TCP_UTILS_H

#include <Arduino.h>
#include "core/mykeyboard.h"

void listenTcpPort(int defaultPort = 0);
void clientTCP();

#endif // TCP_UTILS_H
