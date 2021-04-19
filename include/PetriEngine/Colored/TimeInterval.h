//
// Created by niels on 3/20/19.
//

#ifndef VERIFYPN_TIMEINTERVAL_H
#define VERIFYPN_TIMEINTERVAL_H

#include <limits>
#include <iostream>
#include <algorithm>
#include <map>
#include <cassert>
#include <string>
#include <cctype>
#include <vector>
#include "Colors.h"

namespace PetriEngine {
    namespace Colored{
        class TimeInterval
        {
        public: //Construter
            TimeInterval(Colored::Color color) : leftStrict(false), lowerBound(0), upperBound(std::numeric_limits<int>::max()), rightStrict(true), color(color){ };
            TimeInterval(bool leftStrict, uint32_t lowerBound, uint32_t upperBound, bool rightStrict, Colored::Color color) : leftStrict(leftStrict), lowerBound(lowerBound), upperBound(upperBound), rightStrict(rightStrict), color(color) { };
            TimeInterval(const TimeInterval& ti) : leftStrict(ti.leftStrict), lowerBound(ti.lowerBound), upperBound(ti.upperBound), rightStrict(ti.rightStrict), color(ti.color) {};
            TimeInterval& operator=(const TimeInterval& ti)
            {
                leftStrict = ti.leftStrict;
                lowerBound = ti.lowerBound;
                upperBound = ti.upperBound;
                rightStrict = ti.rightStrict;
                return *this;
            }

            virtual ~TimeInterval() { /* empty */ }
            void divideBoundsBy(int divider);
            const Colored::Color& getColor()  const{
                return color;
            }


        public: // inspectors
            void print(std::ostream& out) const;
            std::string toString();

            inline const uint32_t getLowerBound() const { return lowerBound; }
            inline const uint32_t getUpperBound() const { return upperBound; }
            inline const bool isLowerBoundStrict() const { return leftStrict; }
            inline const bool isUpperBoundStrict() const { return rightStrict; }
            inline bool setUpperBound(uint32_t bound, bool isStrict)
            {
                if(upperBound == bound) rightStrict |= isStrict;
                else if(upperBound > bound){
                    rightStrict = isStrict;
                    upperBound = bound;
                }
                if(upperBound < lowerBound) return false;
                else return true;
            }

            inline const bool isZeroInfinity() const { return !leftStrict && lowerBound == 0 && upperBound == std::numeric_limits<int>::max() && rightStrict; }
            inline const bool contains(uint32_t number) const
            {
                return number >= lowerBound && number <= upperBound;
            }
            inline bool intersects(const TimeInterval& other) const
            {
                assert(!leftStrict);
                assert(!other.leftStrict);
                assert(!rightStrict || upperBound == std::numeric_limits<int>().max());
                assert(!other.rightStrict || other.upperBound == std::numeric_limits<int>().max());
                if(other.lowerBound >= lowerBound &&
                   other.lowerBound <= upperBound) return true;
                return false;
            }

        public: // statics

            static TimeInterval createFor(const std::string& interval,
                                          std::vector<const Colored::Color*> colors, std::unordered_map<std::string, uint32_t> constantValues);
            static inline void ltrim(std::string &s) {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                    return !std::isspace(ch);
                }));
            }
            static Colored::Color createColor(std::vector<const Colored::Color*> colors);

            // trim from end
            static inline void rtrim(std::string &s) {
                s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                    return !std::isspace(ch);
                }).base(), s.end());
            }

            // trim from both ends
            static inline void trim(std::string &s) {
                ltrim(s);
                rtrim(s);
            }

        private: // data
            bool leftStrict;
            uint32_t lowerBound;
            uint32_t upperBound;
            bool rightStrict;
            Colored::Color color;
        };

        inline std::ostream& operator<<(std::ostream& out, const TimeInterval& interval)
        {
            interval.print(out);
            return out;
        }
        // trim from start
    }
}

#endif //VERIFYPN_TIMEINTERVAL_H
