/* PeTe - Petri Engine exTremE
 * Copyright (C) 2011  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                     Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                     Lars Kærlund Østergaard <larsko@gmail.com>,
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
#ifndef PETRINETBUILDER_H
#define PETRINETBUILDER_H

#include <vector>
#include <string>
#include <memory>
#include "TAPNBuilderInterface.h"
#include "PQL/PQL.h"
#include "PetriNet.h"
#include "NetStructures.h"
#include "Colored/ColoredNetStructures.h"
#include "Colored/TimeInvariant.h"
namespace unfoldtacpn {
    /** Builder for building engine representations of PetriNets */
    class PetriNetBuilder : public TAPNBuilderInterface {

    public:
        PetriNetBuilder();
        PetriNetBuilder(const PetriNetBuilder& other);
        void addPlace(const std::string& name, int tokens, double x, double y) override;
        void addPlace(const std::string& name,
                int tokens,
                Colored::TimeInvariant invariant,
                double x = 0,
                double y = 0) override;
        void addTransition(const std::string& name,
                double x,
                double y) override;
        void addTransition(const std::string &name, bool urgent,
                                           double, double) override;
        void addInputArc(const std::string& place,
                const std::string& transition,
                bool inhibitor,
                int weight) override;
        void addInputArc(const std::string &place,
                         const std::string &transition,
                         bool inhibitor, bool transport,
                         int weight,
                         Colored::TimeInterval &interval, std::string transportID) override;

        void addOutputArc(const std::string& transition, const std::string& place, int weight = 1, bool transport = false, std::string transportID = "") override;

        MarkVal const * initMarking()
        {
            return initialMarking.data();
        }

        uint32_t numberOfPlaces() const
        {
            return _placenames.size();
        }

        uint32_t numberOfTransitions() const
        {
            return _transitionnames.size();
        }

        const std::unordered_map<std::string, uint32_t>& getPlaceNames() const
        {
            return _placenames;
        }

        const std::unordered_map<std::string, uint32_t>& getTransitionNames() const
        {
            return _transitionnames;
        }

        std::vector<std::pair<std::string, uint32_t>> orphanPlaces() const {
            std::vector<std::pair<std::string, uint32_t>> res;
            for(uint32_t p = 0; p < _places.size(); p++) {
                if(_places[p].consumers.size() == 0 && _places[p].producers.size() == 0) {
                    for(auto &n : _placenames) {
                        if(n.second == p) {
                            res.push_back(std::make_pair(n.first, initialMarking[p]));
                            break;
                        }
                    }
                }
            }
            return res;
        }

        bool isTimed() { return timed; }

        void setTimed(bool timed) { this->timed = timed;}

        std::map<std::string, std::string> getInvariants() {return _invariantStrings;}

    private:
        bool timed = false;

    protected:
        std::unordered_map<std::string, uint32_t> _placenames;
        std::unordered_map<std::string, uint32_t> _transitionnames;

        std::map<std::string, std::string> _invariantStrings;

        std::vector< std::tuple<double, double> > _placelocations;
        std::vector< std::tuple<double, double> > _transitionlocations;

        std::vector<PetriEngine::Transition> _transitions;
        std::vector<PetriEngine::Place> _places;

        std::vector<MarkVal> initialMarking;
    };

}

#endif // PETRINETBUILDER_H


