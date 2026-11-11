#pragma once

#include "protocol.h"

namespace rf_protocols {

class protocol_linear : public c_rf_protocol {
public:
    protocol_linear() {
        transposition_table[0] = {500, -1500};
        transposition_table[1] = {1500, -500};
        pilot_period = {-15480, 430};
        stop_bit = {1, -21500};
    }
};

} // namespace rf_protocols
