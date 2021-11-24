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
#ifndef PNMLPARSER_H
#define PNMLPARSER_H

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <rapidxml.hpp>

#include "PetriEngine/TAPNBuilderInterface.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/Colored/ColoredNetStructures.h"
#include "PetriEngine/Colored/Expressions.h"
#include "PetriEngine/Colored/Colors.h"
#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"


class PNMLParser {

    struct Arc {
        std::string source,
        target;
        int weight;
        bool transport;
        bool normal;
        bool inhib;
        unfoldtacpn::Colored::ArcExpression_ptr expr;
        std::vector<unfoldtacpn::Colored::TimeInterval> interval;
        std::string transportID;
    };
    typedef std::vector<Arc> ArcList;
    typedef ArcList::iterator ArcIter;

    struct Transition {
        std::string id;
        double x, y;
        unfoldtacpn::Colored::GuardExpression_ptr expr;
        bool urgent;
    };
    typedef std::vector<Transition> TransitionList;
    typedef TransitionList::iterator TransitionIter;

    struct NodeName {
        std::string id;
        bool isPlace;
    };
    typedef std::unordered_map<std::string, NodeName> NodeNameMap;

    typedef std::unordered_map<std::string, unfoldtacpn::Colored::ColorType*> ColorTypeMap;
    typedef std::unordered_map<std::string, unfoldtacpn::Colored::Variable*> VariableMap;

public:

    struct Query {
        std::string name, text;
    };

    PNMLParser() {
        builder = NULL;
    }
    void parse(std::istream& xml,
            unfoldtacpn::ColoredPetriNetBuilder* builder);

    std::vector<Query> getQueries() {
        return queries;
    }

private:
    inline bool stringToBool(std::string b)
    {
        return b == "true" ? 1 : 0;
    }
    void parseElement(rapidxml::xml_node<>* element);
    void parsePlace(rapidxml::xml_node<>* element);
    std::pair<std::string, std::vector<const unfoldtacpn::Colored::Color*>> parseTimeConstraint(rapidxml::xml_node<> *element);
    void parseArc(rapidxml::xml_node<>* element, bool inhibitor = false);
    void parseInterval(rapidxml::xml_node<>* element);
    void parseTransition(rapidxml::xml_node<>* element);
    void parseDeclarations(rapidxml::xml_node<>* element);
    void parseNamedSort(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::ArcExpression_ptr parseArcExpression(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::GuardExpression_ptr parseGuardExpression(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::ColorExpression_ptr parseColorExpression(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::AllExpression_ptr parseAllExpression(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::ColorType* parseUserSort(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::ArcExpression_ptr parseNumberOfExpression(rapidxml::xml_node<>* element);
    void collectColorsInTuple(rapidxml::xml_node<>* element, std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>>& collectedColors);
    unfoldtacpn::Colored::ArcExpression_ptr constructAddExpressionFromTupleExpression(rapidxml::xml_node<>* element,std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> collectedColors, uint32_t numberof);
    std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> cartesianProduct(std::vector<unfoldtacpn::Colored::ColorExpression_ptr> rightSet, std::vector<unfoldtacpn::Colored::ColorExpression_ptr> leftSet);
    std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> cartesianProduct(std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> rightSet, std::vector<unfoldtacpn::Colored::ColorExpression_ptr> leftSet);
    void parseTransportArc(rapidxml::xml_node<>* element);
    void parseValue(rapidxml::xml_node<>* element, std::string& text);
    uint32_t parseNumberConstant(rapidxml::xml_node<>* element);
    void parsePosition(rapidxml::xml_node<>* element, double& x, double& y);
    void parseQueries(rapidxml::xml_node<>* element);
    const unfoldtacpn::Colored::Color* findColor(const char* name) const;
    bool checkIsTimed(rapidxml::xml_node<> *netType);
    unfoldtacpn::ColoredPetriNetBuilder* builder;
    NodeNameMap id2name;
    ArcList arcs;
    TransitionList transitions;
    ColorTypeMap colorTypes;
    VariableMap variables;
    std::vector<Query> queries;
    std::unordered_map<std::string, uint32_t> constantValues;
};

#endif // PNMLPARSER_H

