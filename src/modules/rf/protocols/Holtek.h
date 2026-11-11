#pragma once

#include "protocol.h"

namespace rf_protocols {

class protocol_holtek : public c_rf_protocol {
public:
    protocol_holtek() {
        transposition_table[0] = {-870, 430};
        transposition_table[1] = {-430, 870};
        pilot_period = {-15480, 430};
        stop_bit = {-430, 430};
    }
};

} // namespace rf_protocols
