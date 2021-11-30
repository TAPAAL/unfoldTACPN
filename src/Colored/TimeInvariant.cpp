//
// Created by niels on 3/20/19.
//

#include "Colored/TimeInvariant.h"
#include "Colored/TimeInterval.h"

#include <sstream>

namespace unfoldtacpn {
    namespace Colored {

        TimeInvariant TimeInvariant::createFor(const std::string& invariant, const std::vector<const Colored::Color*>& colors, const std::unordered_map<std::string, uint32_t>& constants){

            bool strict = invariant.find("<=") == std::string::npos;
            int bound = std::numeric_limits<int>().max();

            int pos = strict ? 1 : 2;
            std::string number = invariant.substr(pos);
            TimeInterval::trim(number);

            bool foundInf = invariant.find("inf") != std::string::npos;

            if(foundInf == false) {
                if (constants.find(number) == constants.end())
                    bound = std::stoi(number);
                else
                    bound = constants.find(number)->second;
            }

            Colored::Color color = createColor(colors);
            if (bound == std::numeric_limits<int>().max())
                return TimeInvariant(std::move(color));
            else
                return TimeInvariant(strict, bound, std::move(color));
        }

        Colored::Color TimeInvariant::createColor(const std::vector<const Colored::Color*>& colors) {
            if (colors.size() == 1 ) {
                return Color(colors.front()->getColorType(), colors.front()->getId(), colors.front()->getColorName().c_str());
            } else {
                return Color(colors.front()->getColorType(), colors.front()->getId(), colors);
            }
        }

        void TimeInvariant::print(std::ostream& out) const
        {
            std::string strBound = bound == std::numeric_limits<int>().max() ? "inf" : std::to_string(bound);
            out << (strictComparison ? "<" : "<=") << " " << strBound;
        }

        std::string TimeInvariant::toString() {
            std::stringstream ss;
            print(ss);
            return ss.str();
        }
    }
}