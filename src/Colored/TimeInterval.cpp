//
// Created by niels on 3/20/19.
//
#include "Colored/TimeInterval.h"
#include "Colored/Colors.h"
#include "Colored/TimeInvariant.h"

#include <vector>
#include <sstream>

namespace unfoldtacpn {
    namespace Colored {
        TimeInterval TimeInterval::createFor(const std::string &interval, std::vector<const Colored::Color*> colors, std::unordered_map<std::string, uint32_t> constantValues) {
            Colored::Color color = Colored::TimeInvariant::createColor(colors);
            bool leftStrict = false;
            if(interval.find("(") != std::string::npos){
                leftStrict = true;
            }

            bool rightStrict = false;
            if(interval.find(")") != std::string::npos) {
                rightStrict = true;
            }

            std::string strLoverBound = interval.substr(0, interval.find(","));
            trim(strLoverBound);
            strLoverBound = strLoverBound.substr(1, strLoverBound.size());
            std::string strUpperBound = interval.substr(interval.find(","), interval.size()-1);
            trim(strUpperBound);
            if(rightStrict == true){
                strUpperBound = strUpperBound.substr(1, strUpperBound.find(")")-1);
            } else{
                strUpperBound = strUpperBound.substr(1, strUpperBound.find("]")-1);
            }

            int lowerBound;
            int upperBound;
            if (constantValues.find(strLoverBound) == constantValues.end())
                lowerBound = std::stoi(strLoverBound);
            else
                lowerBound = constantValues[strLoverBound];

            if(strUpperBound.compare("inf") == 0 )
                upperBound = std::numeric_limits<int>().max();
            else {
                if (constantValues.find(strUpperBound) == constantValues.end())
                    upperBound = std::stoi(strUpperBound);
                else
                    upperBound = constantValues[strUpperBound];
            }

            return TimeInterval(leftStrict, lowerBound, upperBound, rightStrict, color);
        }

        void TimeInterval::print(std::ostream &out) const
        {
            std::string strUpperBound = upperBound == std::numeric_limits<int>().max() ? "inf" : std::to_string(upperBound);
            out << color.toString() << " |-> ";
            out << (leftStrict ? "(" : "[") << lowerBound << ","
                << strUpperBound << (rightStrict ? ")" : "]");

        }

        std::string TimeInterval::toString() const {
            std::stringstream ss;
            print(ss);
            return ss.str();
        }
        void TimeInterval::divideBoundsBy(int divider) {
            if(lowerBound != 0 )
                lowerBound = lowerBound / divider;
            if(upperBound != std::numeric_limits<int>().max() ) {
                upperBound = upperBound / divider;
            }
        }
        Colored::Color TimeInterval::createColor(std::vector<const Colored::Color*> colors) {
            if (colors.size() == 1 ) {
                return Color(colors.front()->getColorType(), colors.front()->getId(), colors.front()->getColorName().c_str());
            } else {
                return Color(colors.front()->getColorType(), colors.front()->getId(), colors);
            }
        }
    }
}

