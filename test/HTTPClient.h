#pragma once
#include "Arduino.h"

class HTTPClient {
public:
    void begin(String url);
    int GET();
    void end();
    String getString();
};
