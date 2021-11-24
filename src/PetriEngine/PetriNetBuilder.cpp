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

#include <assert.h>
#include <algorithm>

#include "PetriEngine/PetriNetBuilder.h"
#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PQL/PQLParser.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/PQL/Contexts.h"
#include "PetriEngine/PQL/Expressions.h"

using namespace std;

namespace unfoldtacpn {

    PetriNetBuilder::PetriNetBuilder() : TAPNBuilderInterface(){
    }
    PetriNetBuilder::PetriNetBuilder(const PetriNetBuilder& other)
    : _placenames(other._placenames), _transitionnames(other._transitionnames),
       _placelocations(other._placelocations), _transitionlocations(other._transitionlocations),
       _transitions(other._transitions), _places(other._places),
       initialMarking(other.initialMarking)
    {

    }

    void PetriNetBuilder::addPlace(const string &name, int tokens, double x, double y) {
        if(_placenames.count(name) == 0)
        {
            uint32_t next = _placenames.size();
            _places.emplace_back();
            _placenames[name] = next;
            _placelocations.push_back(std::tuple<double, double>(x,y));
        }

        uint32_t id = _placenames[name];

        while(initialMarking.size() <= id) initialMarking.emplace_back();
        initialMarking[id] = tokens;

    }

    void PetriNetBuilder::addPlace(const string &name, int tokens, Colored::TimeInvariant invariant,
                                   double x, double y) {
        if(_placenames.count(name) == 0){
            uint32_t next = _placenames.size();
            _places.emplace_back();
            _placenames[name] = next;
            _invariantStrings[name] = invariant.toString();
            _placelocations.push_back(std::tuple<double, double>(x,y));

        }
        uint32_t id = _placenames[name];
        while(initialMarking.size() <= id) initialMarking.emplace_back();
            initialMarking[id] = tokens;

    }

    void PetriNetBuilder::addTransition(const string &name,
            double x, double y) {
        if(_transitionnames.count(name) == 0)
        {
            uint32_t next = _transitionnames.size();
            _transitions.emplace_back();
            _transitionnames[name] = next;
            _transitionlocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void PetriNetBuilder::addTransition(const string &name, bool urgent,
                                        double x, double y) {
        if(_transitionnames.count(name) == 0)
        {
            uint32_t next = _transitionnames.size();
            _transitions.emplace_back();
            _transitionnames[name] = next;
            _transitions[next].urgent = urgent;
            _transitionlocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void PetriNetBuilder::addInputArc(const string &place, const string &transition, bool inhibitor, int weight) {
        if(_transitionnames.count(transition) == 0)
        {
            addTransition(transition,0.0,0.0);
        }
        if(_placenames.count(place) == 0)
        {
            addPlace(place,0,0,0);
        }
        uint32_t p = _placenames[place];
        uint32_t t = _transitionnames[transition];

        PetriEngine::Arc arc;
        arc.place = p;
        arc.weight = weight;
        arc.skip = false;
        arc.inhib = inhibitor;
        assert(t < _transitions.size());
        assert(p < _places.size());
        _transitions[t].pre.push_back(arc);
        _transitions[t].inhib |= inhibitor;
        _places[p].consumers.push_back(t);
        _places[p].inhib |= inhibitor;
    }

    void PetriNetBuilder::addInputArc(const std::string &place,
                                      const std::string &transition,
                                      bool inhibitor, bool transport,
                                      int weight,
                                      unfoldtacpn::Colored::TimeInterval &interval, std::string transportID) {
        if(_transitionnames.count(transition) == 0)
        {
            addTransition(transition,0.0,0.0);
        }
        if(_placenames.count(place) == 0)
        {
            //addPlace(place,0,0,0);
        }
        uint32_t p = _placenames[place];
        uint32_t t = _transitionnames[transition];


        uint32_t lowerBound = interval.getLowerBound();
        uint32_t upperBound = interval.getUpperBound();
        bool lowerStrict = interval.isLowerBoundStrict();
        bool upperStrict = interval.isUpperBoundStrict();

        PetriEngine::Arc arc;
        arc.place = p;
        arc.weight = weight;
        arc.skip = false;
        arc.inhib = inhibitor;
        arc.transport = transport;
        arc.lowerBound = lowerBound;
        arc.upperBound = upperBound;
        arc.lowerStrict = lowerStrict;
        arc.upperStrict = upperStrict;
        arc.interval = interval.toString();
        arc.transportID = transportID;
        assert(t < _transitions.size());
        assert(p < _places.size());
        _transitions[t].pre.push_back(arc);
        _transitions[t].inhib |= inhibitor;
        _places[p].consumers.push_back(t);
        _places[p].inhib |= inhibitor;

    }

    void PetriNetBuilder::addOutputArc(const string &transition, const string &place, int weight, bool transport, std::string transportID) {
        if(_transitionnames.count(transition) == 0)
        {
            addTransition(transition,0,0);
        }
        if(_placenames.count(place) == 0)
        {
            addPlace(place,0,0,0);
        }
        uint32_t p = _placenames[place];
        uint32_t t = _transitionnames[transition];

        assert(t < _transitions.size());
        assert(p < _places.size());
        PetriEngine::Arc arc;
        arc.place = p;
        arc.weight = weight;
        arc.skip = false;
        arc.transport = transport;
        arc.transportID = transportID;
        _transitions[t].post.push_back(arc);
        _places[p].producers.push_back(t);
    }

} // PetriEngine

