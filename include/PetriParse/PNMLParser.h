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

#include "TAPNBuilderInterface.h"
#include "PQL/PQL.h"
#include "Colored/ColoredNetStructures.h"
#include "Colored/Expressions.h"
#include "Colored/Colors.h"
#include "Colored/ColoredPetriNetBuilder.h"
#include "Colored/TimeInterval.h"

#include <unordered_map>
#include <map>
#include <unordered_set>
#include <string>
#include <vector>
#include <fstream>
#include <rapidxml.hpp>

namespace unfoldtacpn {
class PNMLParser {
public:

    typedef std::unordered_map<std::string, unfoldtacpn::Colored::ColorType*> ColorTypeMap;
    typedef std::unordered_map<std::string, unfoldtacpn::Colored::Variable*> VariableMap;

public:
    PNMLParser() {
        builder = nullptr;
    }
    void parse(std::istream& xml,
        ColoredPetriNetBuilder* builder);

private:
    int parseWeight(rapidxml::xml_node<>* element);
    unfoldtacpn::Colored::ArcExpression_ptr parseHLInscriptions(rapidxml::xml_node<>* element);
    std::vector<Colored::TimeInterval> parseTimeGuard(rapidxml::xml_node<>* element);
    void parseElement(rapidxml::xml_node<>* element);
    void parsePlace(rapidxml::xml_node<>* element);
    std::pair<std::string, std::vector<const unfoldtacpn::Colored::Color*>> parseTimeConstraint(rapidxml::xml_node<> *element);
    void parseArc(rapidxml::xml_node<>* element, bool inhibitor = false);
    void handleArc(rapidxml::xml_node<>* element);
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
    void parseSingleTransportArc(rapidxml::xml_node<>* element);
    void parseValue(rapidxml::xml_node<>* element, std::string& text);
    uint32_t parseNumberConstant(rapidxml::xml_node<>* element);
    void parsePosition(rapidxml::xml_node<>* element, double& x, double& y);
    const unfoldtacpn::Colored::Color* findColor(const char* name) const;
    unfoldtacpn::ColoredPetriNetBuilder* builder;
    ColorTypeMap colorTypes;
    VariableMap variables;
    std::unordered_map<std::string, uint32_t> constantValues;
    std::map<std::pair<std::string,std::string>, rapidxml::xml_node<>*> transportArcs;
    std::unordered_set<std::string> transitions;
};
}
#endif // PNMLPARSER_H

