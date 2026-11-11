#ifndef WILLY_BLE_SERVICE_H
#define WILLY_BLE_SERVICE_H
#if !defined(LITE_VERSION)
#include <NimBLEServer.h>

class WillyBLEService {
protected:
  NimBLEService *pService = nullptr;
  uint16_t mtu = 23; // default MTU size
public:
  virtual ~WillyBLEService() = default;
  virtual void setup(NimBLEServer *pServer) = 0;
  virtual void end() = 0;
  void setMTU(uint16_t new_mtu) { mtu = new_mtu; }
};
#endif
#endif // WILLY_BLE_SERVICE_H
