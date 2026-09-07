#pragma once
#include "Arduino.h"

class HTTPClient {
public:
    void begin(const String& url) {}
    int GET() { return 200; }
    void end() {}
    String getString() { return ""; }
};
