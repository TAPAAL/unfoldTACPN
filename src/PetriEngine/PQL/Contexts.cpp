/*
 *  Copyright Peter G. Jensen, all rights reserved.
 */

#include "PetriEngine/PQL/Contexts.h"



namespace unfoldtacpn {
    namespace PQL {

        bool ColoredAnalysisContext::resolvePlace(const std::string& place, std::unordered_map<uint32_t, std::string>& out)
        {
            auto it = _coloredPlaceNames.find(place);
            if (it != _coloredPlaceNames.end()) {
                out = it->second;
                return true;
            }
            return false;
        }

        bool ColoredAnalysisContext::resolveTransition(const std::string& transition, std::vector<std::string>& out)
        {
            auto it = _coloredTransitionNames.find(transition);
            if (it != _coloredTransitionNames.end()) {
                out = it->second;
                return true;
            }
            return false;
        }


        AnalysisContext::ResolutionResult AnalysisContext::resolve(const std::string& identifier, bool place)
        {
            ResolutionResult result;
            result.offset = -1;
            result.success = false;
            auto& map = place ? _placeNames : _transitionNames;
            auto it = map.find(identifier);
            if (it != map.end()) {
                result.offset = (int) it->second;
                result.success = true;
                return result;
            }
            return result;
        }
    }
}