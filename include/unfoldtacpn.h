#ifndef LIBUNFOLDTACPN_H
#define LIBUNFOLDTACPN_H

#include "PQL/PQL.h"

#include <vector>
#include <set>

namespace unfoldtacpn {
    class ColoredPetriNetBuilder;
    std::vector<std::pair<PQL::Condition_ptr,std::string>> parse_string_queries(const ColoredPetriNetBuilder& builder, std::istream& qfile);
    std::vector<std::pair<PQL::Condition_ptr, std::string>> parse_xml_queries(const ColoredPetriNetBuilder& builder,
            std::istream& qfile, const std::set<size_t>& to_parse);
}

#endif


