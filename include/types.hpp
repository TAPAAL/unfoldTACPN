#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>

namespace unfoldtacpn::types {
    using InitialTokenAges = std::vector<uint32_t>;
    using InitialMarkingAges = std::unordered_map<uintptr_t,InitialTokenAges>;
}

#endif