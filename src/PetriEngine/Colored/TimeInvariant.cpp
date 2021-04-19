//
// Created by niels on 3/20/19.
//

#include "PetriEngine/Colored/TimeInvariant.h"
#include "PetriEngine/Colored/TimeInterval.h"

namespace PetriEngine {
    namespace Colored {

        TimeInvariant TimeInvariant::createFor(const std::string& invariant, std::vector<const Colored::Color*> colors, std::unordered_map<std::string, uint32_t> constants){
            Colored::Color color = createColor(colors);
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
                    bound = constants[number];
            }


            if (bound == std::numeric_limits<int>().max())
                return TimeInvariant(color);
            else
                return TimeInvariant(strict, bound, color);
        }

        Colored::Color TimeInvariant::createColor(std::vector<const Colored::Color*> colors) {
            if (colors.size() == 1 ) {
                return Color(colors.front()->getColorType(), colors.front()->getId(), colors.front()->getColorName().c_str());
            } else {
                std::cerr << colors.size()  << " colors size";
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