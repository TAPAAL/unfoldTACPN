/* PeTe - Petri Engine exTremE
 * Copyright (C) 2011  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                     Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                     Lars Kærlund Østergaard <larsko@gmail.com>,
 *                     Peter Gjøl Jensen <root@petergjoel.dk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CONTEXTS_H
#define CONTEXTS_H

#include "PQL.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <chrono>
#include <unordered_map>

namespace unfoldtacpn {

    namespace PQL {

        class NamingContext {
        protected:
            const std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>>& _coloredPlaceNames;
            const std::unordered_map<std::string, std::vector<std::string>>& _coloredTransitionNames;
        public:
            NamingContext(  const std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>>& cplaces,
                            const std::unordered_map<std::string, std::vector<std::string>>& ctnames)
                    : _coloredPlaceNames(cplaces),
                      _coloredTransitionNames(ctnames)
                    {}

            bool resolvePlace(const std::string& place, std::unordered_map<uint32_t,std::string>& out);

            bool resolveTransition(const std::string& transition, std::vector<std::string>& out);

            auto& allColoredPlaceNames() const { return _coloredPlaceNames; }
            auto& allColoredTransitionNames() const { return _coloredTransitionNames; }
        };
    } // PQL
}

#endif // CONTEXTS_H
