#pragma once

#include "protocol.h"

namespace rf_protocols {

class protocol_liftmaster : public c_rf_protocol {
public:
    protocol_liftmaster() {
        transposition_table[0] = {800, -400};
        transposition_table[1] = {400, -800};
        pilot_period = {-15480, 430};
        stop_bit = {-3000, 1000};
    }
};

} // namespace rf_protocols
