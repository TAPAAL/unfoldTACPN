/*
 *  Copyright Peter G. Jensen, all rights reserved.
 */

#include "PQL/Contexts.h"
#include "Colored/ColoredPetriNetBuilder.h"

namespace unfoldtacpn {
    namespace PQL {

        bool NamingContext::resolvePlace(const std::string& place, std::unordered_map<uint32_t, std::string>& out)
        {
            auto it = _coloredPlaceNames.find(place);
            if (it != _coloredPlaceNames.end()) {
                out = it->second;
                return true;
            }
            return false;
        }

        bool NamingContext::resolvePlace(const std::string& place, const std::string& color, std::string& out) {
            return _builder && _builder->resolvePlace(place, color, out);
        }

        bool NamingContext::resolveTransition(const std::string& transition, std::vector<std::string>& out)
        {
            auto it = _coloredTransitionNames.find(transition);
            if (it != _coloredTransitionNames.end()) {
                out = it->second;
                return true;
            }
            return false;
        }


    }
}
