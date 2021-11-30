//
// Created by niels on 3/20/19.
//

#ifndef VERIFYPN_TIMEINVARIANT_H
#define VERIFYPN_TIMEINVARIANT_H

#include <string>
#include <map>
#include <limits>
#include <algorithm>
#include <cctype>
#include <locale>
#include "Colors.h"

namespace unfoldtacpn {
    namespace Colored {
        class TimeInvariant {

        public:
            TimeInvariant(Colored::Color&& color) : strictComparison(true), bound(std::numeric_limits<int>::max()), color(std::move(color)) {};
            TimeInvariant(bool strictComparison, int bound, Colored::Color&& color) : strictComparison(strictComparison), bound(bound), color(std::move(color)) { };
            TimeInvariant(const TimeInvariant& ti) : strictComparison(ti.strictComparison), bound(ti.bound), color(ti.color) { };
            TimeInvariant& operator=(const TimeInvariant& ti)
            {
                strictComparison = ti.strictComparison;
                bound = ti.bound;
                return *this;
            }

            virtual ~TimeInvariant(){ /*Left empty*/  };

        public: // inspectors
            void print(std::ostream& out) const;
            std::string toString() const;
            inline const int getBound() const { return bound; }
            inline const bool isBoundStrict() const { return strictComparison; }
            inline const Colored::Color& getColor() const { return color;}

        public: // statics
            static TimeInvariant createFor(const std::string& invariant, const std::vector<const Colored::Color*>& colors, const std::unordered_map<std::string, uint32_t>& constants);
            static Colored::Color createColor(const std::vector<const Colored::Color*>& colors);

        private: // data
            bool strictComparison;
            int bound;
            Colored::Color color;
        };

        inline bool operator==(const TimeInvariant& a, const TimeInvariant& b)
        {
            return a.getBound() == b.getBound() && a.isBoundStrict() == b.isBoundStrict();
        }

        inline bool operator!=(const TimeInvariant& a, const TimeInvariant& b)
        {
            return !(a == b);
        }

        inline std::ostream& operator<<(std::ostream& out, const TimeInvariant& invariant)
        {
            invariant.print(out);
            return out;
        }
        /*
        *  Singleton pattern from:
        * https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
        */
        class TimeInvariantConstant : public TimeInvariant {
        private:
            TimeInvariantConstant();

        public:
            static const TimeInvariant* starColorDotConstant() {
                static TimeInvariantConstant _instance;
                return &_instance;
            }
            TimeInvariantConstant(TimeInvariant const&) = delete;
            void operator=(TimeInvariant const&) = delete;
            bool operator==(const TimeInvariant other) {
                return true;
            }
        };
    }
}

#endif //VERIFYPN_TIMEINVARIANT_H
