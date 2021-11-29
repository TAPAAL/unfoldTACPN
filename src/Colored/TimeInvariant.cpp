//
// Created by niels on 3/20/19.
//

#include "Colored/TimeInvariant.h"
#include "Colored/TimeInterval.h"

namespace unfoldtacpn {
    namespace Colored {

        TimeInvariant TimeInvariant::createFor(const std::string& invariant, const std::vector<const Colored::Color*>& colors, const std::unordered_map<std::string, uint32_t>& constants){
            bool strict = false;

            std::string leq = "<=";
            bool found = invariant.find(leq) != std::string::npos;
            if(!found) {
                strict = true;
            }
            int bound = std::numeric_limits<int>().max();

            int pos = strict ? 1 : 2;
            std::string number = invariant.substr(pos);
            TimeInterval::trim(number);

            bool foundInf = false;
            std::string inf = "inf";
            std::size_t findInf = invariant.find(inf);
            if(findInf != std::string::npos) {
                foundInf = true;
            }

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
            std::string comparison = strictComparison ? "<" : "<=";
            std::string strBound = bound == std::numeric_limits<int>().max() ? "inf" : std::to_string(bound);
            strBound = " " + strBound;
            out << comparison << strBound;
        }
        std::string TimeInvariant::toString() {
            std::string comparison = strictComparison ? "<" : "<=";
            std::string strBound = bound == std::numeric_limits<int>().max() ? "inf" : std::to_string(bound);
            strBound = " " + strBound;
            comparison = comparison + strBound;
            return comparison;
        }
    }
}