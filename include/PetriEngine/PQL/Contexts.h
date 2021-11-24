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

#include "../PetriNet.h"
#include "PQL.h"
#include "../NetStructures.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <chrono>

namespace PetriEngine {

    namespace PQL {

        /** Context provided for context analysis */
        class AnalysisContext {
        protected:
            const std::unordered_map<std::string, uint32_t>& _placeNames;
            const std::unordered_map<std::string, uint32_t>& _transitionNames;
            const PetriNet* _net;
            std::vector<ExprError> _errors;
        public:

            /** A resolution result */
            struct ResolutionResult {
                /** Offset in relevant vector */
                int offset;
                /** True, if the resolution was successful */
                bool success;
            };

            AnalysisContext(const std::unordered_map<std::string, uint32_t>& places, const std::unordered_map<std::string, uint32_t>& tnames, const PetriNet* net)
            : _placeNames(places), _transitionNames(tnames), _net(net) {

            }

            virtual void setHasDeadlock(){};

            const PetriNet* net() const
            {
                return _net;
            }

            /** Resolve an identifier */
            virtual ResolutionResult resolve(const std::string& identifier, bool place = true);

            /** Report error */
            void reportError(const ExprError& error) {
                _errors.push_back(error);
            }

            /** Get list of errors */
            const std::vector<ExprError>& errors() const {
                return _errors;
            }
            auto& allPlaceNames() const { return _placeNames; }
            auto& allTransitionNames() const { return _transitionNames; }

        };

        class ColoredAnalysisContext : public AnalysisContext {
        protected:
            const std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>>& _coloredPlaceNames;
            const std::unordered_map<std::string, std::vector<std::string>>& _coloredTransitionNames;

            bool _colored;

        public:
            ColoredAnalysisContext(const std::unordered_map<std::string, uint32_t>& places,
                                   const std::unordered_map<std::string, uint32_t>& tnames,
                                   const PetriNet* net,
                                   const std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>>& cplaces,
                                   const std::unordered_map<std::string, std::vector<std::string>>& ctnames,
                                   bool colored)
                    : AnalysisContext(places, tnames, net),
                      _coloredPlaceNames(cplaces),
                      _coloredTransitionNames(ctnames),
                      _colored(colored)
            {}

            bool resolvePlace(const std::string& place, std::unordered_map<uint32_t,std::string>& out);

            bool resolveTransition(const std::string& transition, std::vector<std::string>& out);

            bool isColored() const {
                return _colored;
            }
            auto& allColoredPlaceNames() const { return _coloredPlaceNames; }
            auto& allColoredTransitionNames() const { return _coloredTransitionNames; }
        };

        /** Context provided for evalation */
        class EvaluationContext {
        public:

            /** Create evaluation context, this doesn't take ownership */
            EvaluationContext(const MarkVal* marking,
                    const PetriNet* net) {
                _marking = marking;
                _net = net;
            }

            EvaluationContext() {};

            const MarkVal* marking() const {
                return _marking;
            }

            void setMarking(MarkVal* marking) {
                _marking = marking;
            }

            const PetriNet* net() const {
                return _net;
            }
        private:
            const MarkVal* _marking = nullptr;
            const PetriNet* _net = nullptr;
        };

        /** Context for distance computation */
        class DistanceContext : public EvaluationContext {
        public:

            DistanceContext(const PetriNet* net,
                    const MarkVal* marking)
            : EvaluationContext(marking, net) {
                _negated = false;
            }


            void negate() {
                _negated = !_negated;
            }

            bool negated() const {
                return _negated;
            }

        private:
            bool _negated;
        };

        /** Context for condition to TAPAAL export */
        class TAPAALConditionExportContext {
        public:
            bool failed;
            std::string netName;
        };
    } // PQL
} // PetriEngine

#endif // CONTEXTS_H
