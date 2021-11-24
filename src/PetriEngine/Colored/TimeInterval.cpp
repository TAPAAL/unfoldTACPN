//
// Created by niels on 3/20/19.
//
#include "PetriEngine/Colored/TimeInterval.h"
#include "PetriEngine/Colored/Colors.h"
#include "PetriEngine/Colored/TimeInvariant.h"
#include <vector>

namespace unfoldtacpn {
    namespace Colored {
        TimeInterval TimeInterval::createFor(const std::string &interval, std::vector<const Colored::Color*> colors, std::unordered_map<std::string, uint32_t> constantValues) {
            Colored::Color color = Colored::TimeInvariant::createColor(colors);
            bool leftStrict = false;
            std::string leftParentheses = "(";
            std::size_t foundLeft = interval.find(leftParentheses);
            if(foundLeft != std::string::npos){
                leftStrict = true;
            }

            bool rightStrict = false;
            std::string rightParentheses = ")";
            std::size_t foundRight = interval.find(rightParentheses);
            if(foundRight != std::string::npos) {
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
            std::string leftParenthesis = leftStrict ? "(" : "[";
            std::string rightParenthesis = rightStrict ? ")" : "]";
            std::string strLowerBound = std::to_string(lowerBound);
            std::string strUpperBound = upperBound == std::numeric_limits<int>().max() ? "inf" : std::to_string(upperBound);

            out << leftParenthesis << strLowerBound << "," << strUpperBound << rightParenthesis;
        }

        std::string TimeInterval::toString() {
            std::string leftParenthesis = leftStrict ? "(" : "[";
            std::string rightParenthesis = rightStrict ? ")" : "]";
            std::string strLowerBound = std::to_string(lowerBound);
            std::string strUpperBound = upperBound == std::numeric_limits<int>().max() ? "inf" : std::to_string(upperBound);

            return leftParenthesis + strLowerBound + "," + strUpperBound + rightParenthesis;
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

