#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <vector>

namespace rf_protocols {

class c_rf_protocol {
public:
    std::vector<int> transposition_table[2];
    std::vector<int> pilot_period;
    std::vector<int> stop_bit;

    c_rf_protocol() = default;
    virtual ~c_rf_protocol() = default;
};

} // namespace rf_protocols

#endif
