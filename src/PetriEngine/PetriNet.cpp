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
#include "PetriEngine/PetriNet.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/PQL/Contexts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <string>

using namespace std;

namespace unfoldtacpn {

    PetriNet::PetriNet(uint32_t trans, uint32_t invariants, uint32_t places)
    : _ninvariants(invariants), _ntransitions(trans), _nplaces(places),
            _transitions(_ntransitions+1),
            _invariants(_ninvariants),            
            _placeToPtrs(_nplaces+1) {

        // to avoid special cases
        _transitions[_ntransitions].inputs = _ninvariants;
        _transitions[_ntransitions].outputs = _ninvariants;
        _placeToPtrs[_nplaces] = _ntransitions;
        _initialMarking = new MarkVal[_nplaces];
//        assert(_nplaces > 0);
    }

    PetriNet::~PetriNet() {
        delete[] _initialMarking;
    }

    int PetriNet::inArc(uint32_t place, uint32_t transition) const
    {
        assert(_nplaces > 0);
        assert(place < _nplaces);
        assert(transition < _ntransitions);
        
        uint32_t imin = _transitions[transition].inputs;
        uint32_t imax = _transitions[transition].outputs;
        if(imin == imax)
        {
            // NO INPUT!
            return 0;
        }
        
        for(;imin < imax; ++imin)
        {
            const Invariant& inv = _invariants[imin];
            if(inv.place == place)
            {
                return inv.inhibitor ? 0 :  _invariants[imin].tokens;
            }
        }
        return 0;
    }
    int PetriNet::outArc(uint32_t transition, uint32_t place) const
    {
        assert(_nplaces > 0);
        assert(place < _nplaces);
        assert(transition < _ntransitions);
        
        uint32_t imin = _transitions[transition].outputs;
        uint32_t imax = _transitions[transition+1].inputs;
        for(;imin < imax; ++imin)
        {
            if(_invariants[imin].place == place) return _invariants[imin].tokens;
        }
        return 0;   
    }
    
    bool PetriNet::deadlocked(const MarkVal* m) const {
        //Check that we can take from the marking
        if(_nplaces == 0)
        {
            return _ntransitions == 0;
        }
        for (size_t i = 0; i < _nplaces; i++) {
            if(i == 0 || m[i] > 0) // orphans are currently under "place 0" as a special case
            {
                uint32_t first = _placeToPtrs[i];
                uint32_t last = _placeToPtrs[i+1];
                for(;first != last; ++first)
                {
                    const TransPtr& ptr = _transitions[first];
                    uint32_t finv = ptr.inputs;
                    uint32_t linv = ptr.outputs;
                    bool allgood = true;
                    for(;finv != linv; ++finv)
                    {
                        if(!_invariants[finv].inhibitor){
                            allgood &= m[_invariants[finv].place] >= _invariants[finv].tokens;
                        } else {
                            allgood &= m[_invariants[finv].place] < _invariants[finv].tokens;
                        }
                        if(!allgood){
                            break;
                        }
                    }
                    
                    if(allgood) 
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
    
    std::pair<const Invariant*, const Invariant*> PetriNet::preset(uint32_t id) const
    {
        const TransPtr& transition = _transitions[id];
        uint32_t first = transition.inputs;
        uint32_t last = transition.outputs;
        return std::make_pair(&_invariants[first], &_invariants[last]);
    }

    std::pair<const Invariant*, const Invariant*> PetriNet::postset(uint32_t id) const
    {
        uint32_t first = _transitions[id].outputs;
        uint32_t last = _transitions[id+1].inputs;
        return std::make_pair(&_invariants[first], &_invariants[last]);
    }
    
    bool PetriNet::fireable(const MarkVal *marking, int transitionIndex)
    {
        const TransPtr& transition = _transitions[transitionIndex];
        uint32_t first = transition.inputs;
        uint32_t last = transition.outputs;

        for(;first < last; ++first){
            const Invariant& inv = _invariants[first];
            if(inv.inhibitor == (marking[inv.place] >= inv.tokens))
                return false;
        }
        return true;
    }

    MarkVal PetriNet::initial(size_t id) const {
        return _initialMarking[id];
    }

    MarkVal* PetriNet::makeInitialMarking() const
    {
        MarkVal* marking = new MarkVal[_nplaces];
        std::copy(_initialMarking, _initialMarking+_nplaces, marking);
        return marking;
    }
    
    void PetriNet::sort()
    {
        for(size_t i = 0; i < _ntransitions; ++i)
        {
            TransPtr& t = _transitions[i];
            std::sort(&_invariants[t.inputs], &_invariants[t.outputs], [](const auto& a, const auto& b) { return a.place < b.place; });
            TransPtr& t2 = _transitions[i + 1];
            std::sort(&_invariants[t.outputs], &_invariants[t2.inputs], [](const auto& a, const auto& b) { return a.place < b.place; });            
        }
    }
    
    void PetriNet::toXML(std::ostream& out, bool timed, std::map<std::string, std::string> _invariantStrings, bool verifydtapn)
    {
        if(!timed) {
            out << "<?xml version=\"1.0\"?>\n"
                << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n" 
                << "<net id=\"ClientsAndServers-PT-N0500P0\" type=\"http://www.pnml.org/version-2009/grammar/ptnet\">\n";
            out << "<page id=\"page0\">\n" 
                << "<name>\n" 
                << "<text>DefaultPage</text>" 
                << "</name>";
        } else {
            if (!verifydtapn) {
                out << "<?xml version=\"1.0\"?>\n";
            }

            out << "<pnml xmlns=\"http://www.pnml.org/version-2009/grammar/pnml\">\n"
                << "<net id=\"ClientsAndServers-PT-N0500P0\" active=\"true\" type=\"P/T net\">\n";


        }

        for(size_t i = 0; i < _nplaces; ++i)
        {
            auto& p = _placenames[i];
            auto& placelocation = _placelocations[i];
            if(!timed) {
                out << "<place id=\"" << p << "\">\n"
                    << "<graphics><position x=\"" << get<0>(placelocation) 
                    << "\" y=\"" << get<1>(placelocation) << "\"/></graphics>\n"
                    << "<name><text>" << p << "</text></name>\n";
                if(_initialMarking[i] > 0)
                {
                    out << "<initialMarking><text>" << _initialMarking[i] << "</text></initialMarking>\n";
                }
                out << "</place>\n";
            } else {
                std::string invariant = _invariantStrings.at(p);
                std::string toReplace("<");
                size_t pos = invariant.find(toReplace);
                invariant.replace(pos, toReplace.length(), "&lt;");

                if (verifydtapn) {
                    out << R"(<place id=")" << p << "\"" << R"( initialMarking=")" << _initialMarking[i] << R"(" invariant=")" << invariant <<
                        R"(" name=")" << p << "\"/>\n";
                }
                else {
                    out << R"(<place displayName="true" id=")" << p << "\"" << R"( initialMarking=")" << _initialMarking[i] << R"(" invariant=")" << invariant <<
                        R"(" markingOffsetX="0.0" markingOffsetY="0.0" name=")" << p << R"(" nameOffsetX="-5.0" nameOffsetY="35.0" positionX=")" << get<0>(placelocation) <<  R"(" positionY=")" << get<1>(placelocation) << "\" />\n";
                }
            }
        }
        for(size_t i = 0; i < _ntransitions; ++i)
        {
            auto& transitionlocation = _transitionlocations[i];
            if (!timed) {
                out << "<transition id=\"" << _transitionnames[i] << "\">\n"
                    << "<name><text>" << _transitionnames[i] << "</text></name>\n";
                out << "<graphics><position x=\"" << get<0>(transitionlocation) 
                    << "\" y=\"" << get<1>(transitionlocation) << "\"/></graphics>\n";
                out << "</transition>\n";
            } else {
                if (verifydtapn) {
                    out << R"(<transition id=")" << _transitionnames[i] << "\" name=\"" << _transitionnames[i]  << R"(" urgent=")" << BoolToString(_transitions[i].urgent) << "\"";
                    out << "/>\n";
                }
                else {
                    out << R"(<transition angle="0" displayName="true" id=")" << _transitionnames[i] << "\" name=\"" << _transitionnames[i]  << R"(" infiniteServer="false" nameOffsetX="-5.0" nameOffsetY="35.0" positionX=")" << get<0>(transitionlocation) << R"(" positionY=")" << get<1>(transitionlocation) << R"(" priority="0"
                urgent=")" << BoolToString(_transitions[i].urgent) << " \"";
                    out << "/>\n";
                }
            }
        }
        size_t id = 0;
        for(size_t t = 0; t < _ntransitions; ++t)
        {
            auto pre = preset(t);

            for(; pre.first != pre.second; ++pre.first)
            {
                if(!timed) {
                    out << "<arc id=\"" << (id++) << "\" source=\""
                        << _placenames[pre.first->place] << "\" target=\""
                        << _transitionnames[t]
                        << "\" type=\""
                        << (pre.first->inhibitor ? "inhibitor" : "normal")
                        << "\">\n";
                    
                    if(pre.first->tokens > 1)
                    {
                        out << "<inscription><text>" << pre.first->tokens << "</text></inscription>\n";                    
                    }
                    
                    out << "</arc>\n";
                } else {
                    std::string type = getType(pre, true);
                    std::string arcName = getArcIdentifier(type);
                    if (verifydtapn) {
                        if (arcName.compare("transportArc") == 0) {
                            auto post = postset(t);
                            for (; post.first != post.second; ++post.first) {
                                if (post.first->transportID.compare(pre.first->transportID) == 0 && (getType(post, false).compare("transport")) == 0) {
                                    out << "<" << arcName << R"( id=")" << (id++) << R"(" inscription=")" << pre.first->interval << R"(" source=")" <<   _placenames[pre.first->place] << R"(" transition=")" << _transitionnames[t] <<  "\" weight=\"" << pre.first->tokens << "\" target=\"" << _placenames[post.first->place] << "\" />\n";
                                    continue;
                                }
                            }
                        } else {
                            std::string interval = pre.first->interval.empty() ? "[0,inf)" : pre.first->interval;
                            out << "<" << arcName << R"( id=")" << (id++) << R"(" inscription=")" << interval << R"(" source=")" <<   _placenames[pre.first->place] << "\" weight=\"" << pre.first->tokens << "\" target=\"" << _transitionnames[t] << "\" />\n";
                        }
                    }
                    else {
                        if (arcName.compare("transportArc") == 0) {
                            auto post = postset(t);
                            for (; post.first != post.second; ++post.first) {
                                if (post.first->transportID.compare(pre.first->transportID) == 0 && (getType(post, false).compare("transport")) == 0) {
                                    out << R"(<arc id=")" << (id++) << R"(" inscription=")" << pre.first->interval << R"(" nameOffsetX="0.0" source=")" <<   _placenames[pre.first->place] << "\" weight=\"" << pre.first->tokens << "\" target=\"" << _transitionnames[t] << "\" type=\"" << type << "\" />\n";
                                    out << R"(<arc id=")" << (id++) << R"(" inscription=")" << pre.first->interval
                                    << R"(" nameOffsetX="0.0" source=")" << _transitionnames[t] << "\" weight=\"" << post.first->tokens << "\" target=\"" <<  _placenames[post.first->place] << "\" type=\"" << type << "\" />\n";
                                    continue;
                                }
                            }
                        }
                        else {
                            std::string interval = pre.first->interval.empty() ? "[0,inf)" : pre.first->interval;
                            out << R"(<arc id=")" << (id++) << R"(" inscription=")" << interval << R"(" nameOffsetX="0.0" source=")" <<   _placenames[pre.first->place] << "\" weight=\"" << pre.first->tokens << "\" target=\"" << _transitionnames[t] << "\" type=\"" << type << "\" />\n";
                        }

                    }
                }
            }

            auto post = postset(t);
            for(; post.first != post.second; ++post.first)
            {
                if (!timed) {
                    out << "<arc id=\"" << (id++) << "\" source=\""
                        << _transitionnames[t] << "\" target=\""
                        << _placenames[post.first->place] << "\">\n";
                    
                    if(post.first->tokens > 1)
                    {
                        out << "<inscription><text>" << post.first->tokens << "</text></inscription>\n";                    
                    }
                    
                    out << "</arc>\n";
                }
                else {
                    std::string type = getType(post, false);
                    if(type.compare("transport") == 0)
                        continue;
                    if (verifydtapn) {
                        std::string arcName = getArcIdentifier(type);
                        out << "<" << arcName <<  " id=\"" << (id++) << "\" source=\""
                            << _transitionnames[t] << "\" target=\""
                            << _placenames[post.first->place] << "\" weight=\"" << post.first->tokens << "\"/>\n";
                    } else {
                        out << "<arc id=\"" << (id++) << "\" source=\""
                            << _transitionnames[t] << "\" target=\""
                            << _placenames[post.first->place] << R"(" type=")" << type << "\" weight=\"" << post.first->tokens << "\"/>\n";
                    }
                }
            }
        }
        if (!timed)
            out << "</page></net>\n</pnml>";
        else
            out << "</net>\n</pnml>";
    }

    std::string PetriNet::getType(std::pair<const Invariant*, const Invariant*> pair, bool pre) {
        if(pre) {
            if (pair.first->transport)
                return "transport";
            else if (pair.first->inhibitor)
                return "inhibitor";
            else
                return "timed";
        } else {
            if (pair.first->transport)
                return "transport";
            else
                return "normal";
        }
    }

    std::string PetriNet::getArcIdentifier(std::string arcType) {
        std::string arcIdentifier;
        if (arcType.compare("normal") == 0) {
            arcIdentifier = "outputArc";
        } else if (arcType.compare("timed") == 0 ) {
            arcIdentifier = "inputArc";
        } else if (arcType.compare("transport") == 0) {
            arcIdentifier = "transportArc";
        } else {
            arcIdentifier = "inhibitorArc";
        }
        return arcIdentifier;
    }
    
} // PetriEngine

