#ifndef LIBUNFOLDTACPN_H
#define LIBUNFOLDTACPN_H

#include "PQL/PQL.h"

#include <vector>
#include <set>

namespace unfoldtacpn {
    class ColoredPetriNetBuilder;
    std::vector<PQL::Condition_ptr> readStringQueries(ColoredPetriNetBuilder& builder, std::ifstream& qfile);
    std::vector<PQL::Condition_ptr> readXMLQueries(ColoredPetriNetBuilder& builder,
            std::ifstream& qfile, const std::set<size_t>& to_parse);
}

#endif


