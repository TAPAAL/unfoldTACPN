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

#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <istream>
#include <cstring>


#include "PetriParse/PNMLParser.h"
#include "errorcodes.h"

using namespace unfoldtacpn;
using namespace std;
using namespace unfoldtacpn::PQL;

void PNMLParser::parse(istream& xml,
        ColoredPetriNetBuilder* builder) {
    //Clear any left overs
    id2name.clear();
    arcs.clear();
    transitions.clear();
    colorTypes.clear();

    //Set the builder
    this->builder = builder;

    //Parser the xml
    rapidxml::xml_document<> doc;
    vector<char> buffer((istreambuf_iterator<char>(xml)), istreambuf_iterator<char>());
    buffer.push_back('\0');
    doc.parse<0>(&buffer[0]);

    rapidxml::xml_node<>* root = doc.first_node();
    if(strcmp(root->name(), "pnml") != 0)
    {
        std::cerr << "expecting <pnml> tag as root-node in xml tree." << std::endl;
        exit(ErrorCode);
    }

    auto declarations = root->first_node("declaration");
    if(declarations == nullptr){
        declarations = root->first_node("net")->first_node("declaration");
    }

    if (declarations) {
        parseDeclarations(declarations);
    }

    auto netType = root->first_node("net");
    rapidxml::xml_node<> *placeElement = netType->first_node("place");
    while(placeElement == nullptr && netType != nullptr){
        netType = netType->first_node();
        placeElement = netType->first_node("place");
    }

    //xml_node<> *child = node->first_node(); child; child = child->next_sibling()
    for (auto it = root->first_node(); it; it = it->next_sibling()) {
        if (strcmp(it->name(), "constant") == 0)
            constantValues[it->first_attribute("name")->value()] = atoi(it->first_attribute("value")->value());
         else
            break;
    }
    parseElement(root);

    //Add all the transition
    for (auto & transition : transitions)
        builder->addTransition(transition.id, transition.expr, transition.urgent, transition.x, transition.y);

    //Add all the arcs
    for (auto & arc : arcs) {
        auto a = arc;

        //Check that source id exists
        if (id2name.find(arc.source) == id2name.end()) {
            fprintf(stderr,
                    "XML Parsing error: Arc source with id=\"%s\" wasn't found!\n",
                    arc.source.c_str());
            continue;
        }
        //Check that target id exists
        if (id2name.find(arc.target) == id2name.end()) {
            fprintf(stderr,
                    "XML Parsing error: Arc target with id=\"%s\" wasn't found!\n",
                    arc.target.c_str());
            continue;
        }
        //Find source and target
        NodeName source = id2name[arc.source];
        NodeName target = id2name[arc.target];

        if (source.isPlace && !target.isPlace) {
            builder->addInputArc(source.id, target.id, arc.inhib, arc.transport, arc.expr, arc.interval, arc.weight, arc.transportID);
        } else if (!source.isPlace && target.isPlace) {
            builder->addOutputArc(source.id, target.id, arc.expr, arc.transport, arc.transportID);
        } else {
            fprintf(stderr,
                    "XML Parsing error: Arc from \"%s\" to \"%s\" is neither input nor output!\n",
                    source.id.c_str(),
                    target.id.c_str());
        }
    }
    //Unset the builder
    this->builder = nullptr;

    //Cleanup
    id2name.clear();
    arcs.clear();
    transitions.clear();
    colorTypes.clear();
}

bool PNMLParser::checkIsTimed(rapidxml::xml_node<> *netType){
    rapidxml::xml_node<char> * placeElement = netType->first_node("place");
    rapidxml::xml_node<char> * arcElement = netType->first_node("inputArc");
    while(placeElement == nullptr && netType != nullptr){
        netType = netType->first_node();
        if(netType != nullptr){
            placeElement = netType->first_node("place");
        }
    }
    while(arcElement == nullptr && netType != nullptr){
        netType = netType->first_node();
        if(netType != nullptr){
            arcElement = netType->first_node("inputArc");
        }
    }
    if(placeElement != nullptr){
        if(placeElement->first_attribute("invariant") == nullptr){
            return false;
        }
    }
    if(arcElement != nullptr){
        if(arcElement->first_attribute("inscription") == nullptr){
            return false;
        }
    }

    return true;
}

void PNMLParser::parseDeclarations(rapidxml::xml_node<>* element) {
    for (auto it = element->first_node(); it; it = it->next_sibling()) {
        if (strcmp(it->name(), "namedsort") == 0) {
            parseNamedSort(it);
        } else if (strcmp(it->name(), "variabledecl") == 0) {
            auto var = new unfoldtacpn::Colored::Variable {
                it->first_attribute("id")->value(),
                parseUserSort(it)
            };
            variables[it->first_attribute("id")->value()] = var;
        } else {
            parseDeclarations(element->first_node());
        }
    }
}

bool isInitialBinding(std::vector<const unfoldtacpn::Colored::Color*>& binding) {
    for (auto color : binding) {
        if (color->getId() != 0)
            return false;
    }
    return true;
}

void PNMLParser::parseNamedSort(rapidxml::xml_node<>* element) {
    auto type = element->first_node();
    auto ct = strcmp(type->name(), "productsort") == 0 ?
              new unfoldtacpn::Colored::ProductType(std::string(element->first_attribute("id")->value())) :
              new unfoldtacpn::Colored::ColorType(std::string(element->first_attribute("id")->value()));

    if (strcmp(type->name(), "dot") == 0) {
        ct->addDot();
    } else if (strcmp(type->name(), "productsort") == 0) {
        for (auto it = type->first_node(); it; it = it->next_sibling()) {
            if (strcmp(it->name(), "usersort") == 0) {
                ((unfoldtacpn::Colored::ProductType*)ct)->addType(colorTypes[it->first_attribute("declaration")->value()]);
            }
        }
    } else {
        for (auto it = type->first_node(); it; it = it->next_sibling()) {
            auto id = it->first_attribute("id");
            assert(id != nullptr);
            ct->addColor(id->value());
        }
    }

    std::string id = element->first_attribute("id")->value();
    colorTypes[id] = ct;
    builder->addColorType(id, ct);
}

unfoldtacpn::Colored::ArcExpression_ptr PNMLParser::parseArcExpression(rapidxml::xml_node<>* element) {
    if (strcmp(element->name(), "numberof") == 0) {
        return parseNumberOfExpression(element);
    } else if (strcmp(element->name(), "add") == 0) {
        std::vector<unfoldtacpn::Colored::ArcExpression_ptr> constituents;
        for (auto it = element->first_node(); it; it = it->next_sibling()) {
            constituents.push_back(parseArcExpression(it));
        }
        return std::make_shared<unfoldtacpn::Colored::AddExpression>(std::move(constituents));
    } else if (strcmp(element->name(), "subtract") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        auto res = std::make_shared<unfoldtacpn::Colored::SubtractExpression>(parseArcExpression(left), parseArcExpression(right));
        auto next = right;
        while ((next = next->next_sibling())) {
            res = std::make_shared<unfoldtacpn::Colored::SubtractExpression>(res, parseArcExpression(next));
        }
        return res;
    } else if (strcmp(element->name(), "scalarproduct") == 0) {
        auto scalar = element->first_node();
        auto ms = scalar->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::ScalarProductExpression>(parseArcExpression(ms), parseNumberConstant(scalar));
    } else if (strcmp(element->name(), "all") == 0) {
        return parseNumberOfExpression(element->parent());
    } else if (strcmp(element->name(), "subterm") == 0 || strcmp(element->name(), "structure") == 0) {
        return parseArcExpression(element->first_node());
    }
    printf("Could not parse '%s' as an arc expression\n", element->name());
    assert(false);
    return nullptr;
}

unfoldtacpn::Colored::GuardExpression_ptr PNMLParser::parseGuardExpression(rapidxml::xml_node<>* element) {
    if (strcmp(element->name(), "lt") == 0 || strcmp(element->name(), "lessthan") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::LessThanExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "gt") == 0 || strcmp(element->name(), "greaterthan") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::GreaterThanExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "leq") == 0 || strcmp(element->name(), "lessthanorequal") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::LessThanEqExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "geq") == 0 || strcmp(element->name(), "greaterthanorequal") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::GreaterThanEqExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "eq") == 0 || strcmp(element->name(), "equality") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::EqualityExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "neq") == 0 || strcmp(element->name(), "inequality") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::InequalityExpression>(parseColorExpression(left), parseColorExpression(right));
    } else if (strcmp(element->name(), "not") == 0) {
        return std::make_shared<unfoldtacpn::Colored::NotExpression>(parseGuardExpression(element->first_node()));
    } else if (strcmp(element->name(), "and") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::AndExpression>(parseGuardExpression(left), parseGuardExpression(right));
    } else if (strcmp(element->name(), "or") == 0) {
        auto left = element->first_node();
        auto right = left->next_sibling();
        return std::make_shared<unfoldtacpn::Colored::OrExpression>(parseGuardExpression(left), parseGuardExpression(right));
    } else if (strcmp(element->name(), "subterm") == 0 || strcmp(element->name(), "structure") == 0) {
        return parseGuardExpression(element->first_node());
    }

    printf("Could not parse '%s' as a guard expression\n", element->name());
    assert(false);
    return nullptr;
}

unfoldtacpn::Colored::ColorExpression_ptr PNMLParser::parseColorExpression(rapidxml::xml_node<>* element) {
    if (strcmp(element->name(), "dotconstant") == 0) {
        return std::make_shared<unfoldtacpn::Colored::DotConstantExpression>();
    } else if (strcmp(element->name(), "variable") == 0) {
        return std::make_shared<unfoldtacpn::Colored::VariableExpression>(variables[element->first_attribute("refvariable")->value()]);
    } else if (strcmp(element->name(), "useroperator") == 0) {
        return std::make_shared<unfoldtacpn::Colored::UserOperatorExpression>(findColor(element->first_attribute("declaration")->value()));
    } else if (strcmp(element->name(), "successor") == 0) {
        return std::make_shared<unfoldtacpn::Colored::SuccessorExpression>(parseColorExpression(element->first_node()));
    } else if (strcmp(element->name(), "predecessor") == 0) {
        return std::make_shared<unfoldtacpn::Colored::PredecessorExpression>(parseColorExpression(element->first_node()));
    } else if (strcmp(element->name(), "tuple") == 0) {
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> colors;
        for (auto it = element->first_node(); it; it = it->next_sibling()) {
            colors.push_back(parseColorExpression(it));
        }
        return std::make_shared<unfoldtacpn::Colored::TupleExpression>(std::move(colors));
    } else if (strcmp(element->name(), "subterm") == 0 || strcmp(element->name(), "structure") == 0) {
        return parseColorExpression(element->first_node());
    }
    assert(false);
    return nullptr;
}

unfoldtacpn::Colored::AllExpression_ptr PNMLParser::parseAllExpression(rapidxml::xml_node<>* element) {
    if (strcmp(element->name(), "all") == 0) {
        return std::make_shared<unfoldtacpn::Colored::AllExpression>(parseUserSort(element));
    } else if (strcmp(element->name(), "subterm") == 0) {
        return parseAllExpression(element->first_node());
    }

    return nullptr;
}

unfoldtacpn::Colored::ColorType* PNMLParser::parseUserSort(rapidxml::xml_node<>* element) {
    if (element) {
        for (auto it = element->first_node(); it; it = it->next_sibling()) {
            if (strcmp(it->name(), "usersort") == 0) {
                return colorTypes[it->first_attribute("declaration")->value()];
            } else if (strcmp(it->name(), "structure") == 0
                    || strcmp(it->name(), "type") == 0
                    || strcmp(it->name(), "subterm") == 0) {
                return parseUserSort(it);
            }
        }
    }
    assert(false);
    return nullptr;
}

unfoldtacpn::Colored::ArcExpression_ptr PNMLParser::parseNumberOfExpression(rapidxml::xml_node<>* element) {
    auto num = element->first_node();
    uint32_t number = parseNumberConstant(num);
    rapidxml::xml_node<>* first;
    if (number) {
        first = num->next_sibling();
    } else {
        number = 1;
        first = num;
    }

    if(strcmp(first->first_node()->name(), "tuple") == 0){
        std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> collectedColors;
        collectColorsInTuple(first->first_node(), collectedColors);
        return constructAddExpressionFromTupleExpression(first->first_node(), collectedColors, number);
    }

    auto allExpr = parseAllExpression(first);
    if (allExpr) {
        return std::make_shared<unfoldtacpn::Colored::NumberOfExpression>(std::move(allExpr), number);
    } else {
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> colors;
        for (auto it = first; it; it = it->next_sibling()) {
            colors.push_back(parseColorExpression(it));
        }
        return std::make_shared<unfoldtacpn::Colored::NumberOfExpression>(std::move(colors), number);
    }
}

void PNMLParser::collectColorsInTuple(rapidxml::xml_node<>* element, std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>>& collectedColors){
    if (strcmp(element->name(), "tuple") == 0) {
        for (auto it = element->first_node(); it; it = it->next_sibling()) {
            collectColorsInTuple(it->first_node(), collectedColors);
        }
    } else if (strcmp(element->name(), "all") == 0) {
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> expressionsToAdd;
        auto expr = parseAllExpression(element);
        std::unordered_map<uint32_t, std::vector<const unfoldtacpn::Colored::Color *>> constantMap;
        uint32_t index = 0;
        expr->getConstants(constantMap, index);
        for(auto positionColors : constantMap){
            for(auto color : positionColors.second){
                expressionsToAdd.push_back(std::make_shared<unfoldtacpn::Colored::UserOperatorExpression>(color));
            }
        }
        collectedColors.push_back(expressionsToAdd);
        return;
    } else if (strcmp(element->name(), "subterm") == 0 || strcmp(element->name(), "structure") == 0) {
        collectColorsInTuple(element->first_node(), collectedColors);
    } else if (strcmp(element->name(), "useroperator") == 0 || strcmp(element->name(), "dotconstant") == 0 || strcmp(element->name(), "variable") == 0
                    || strcmp(element->name(), "successor") == 0 || strcmp(element->name(), "predecessor") == 0) {
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> expressionsToAdd;
        auto color = parseColorExpression(element);
        expressionsToAdd.push_back(color);
        collectedColors.push_back(expressionsToAdd);
        return;
    } else{
        printf("Could not collect tuple colors from arc expression '%s'\n", element->name());
        return;
    }
}

unfoldtacpn::Colored::ArcExpression_ptr PNMLParser::constructAddExpressionFromTupleExpression(rapidxml::xml_node<>* element,std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> collectedColors, uint32_t numberof){
    auto initCartesianSet = cartesianProduct(collectedColors[0], collectedColors[1]);
    for(uint32_t i = 2; i < collectedColors.size(); i++){
        initCartesianSet = cartesianProduct(initCartesianSet, collectedColors[i]);
    }
    std::vector<unfoldtacpn::Colored::NumberOfExpression_ptr> numberOfExpressions;
    for(auto set : initCartesianSet){
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> colors;
        for (auto color : set) {
            colors.push_back(color);
        }
        std::shared_ptr<unfoldtacpn::Colored::TupleExpression> tupleExpr = std::make_shared<unfoldtacpn::Colored::TupleExpression>(std::move(colors));
        tupleExpr->setColorType(tupleExpr->getColorType(colorTypes));
        std::vector<unfoldtacpn::Colored::ColorExpression_ptr> placeholderVector;
        placeholderVector.push_back(tupleExpr);
        numberOfExpressions.push_back(std::make_shared<unfoldtacpn::Colored::NumberOfExpression>(std::move(placeholderVector),numberof));
    }
    std::vector<unfoldtacpn::Colored::ArcExpression_ptr> constituents;
    for (auto expr : numberOfExpressions) {
        constituents.push_back(expr);
    }
    return std::make_shared<unfoldtacpn::Colored::AddExpression>(std::move(constituents));
}

std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> PNMLParser::cartesianProduct(std::vector<unfoldtacpn::Colored::ColorExpression_ptr> rightSet, std::vector<unfoldtacpn::Colored::ColorExpression_ptr> leftSet){
    std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> returnSet;
    for(auto expr : rightSet){
        for(auto expr2 : leftSet){
            std::vector<unfoldtacpn::Colored::ColorExpression_ptr> toAdd;
            toAdd.push_back(expr);
            toAdd.push_back(expr2);
            returnSet.push_back(toAdd);
        }
    }
    return returnSet;
}
std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> PNMLParser::cartesianProduct(std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> rightSet, std::vector<unfoldtacpn::Colored::ColorExpression_ptr> leftSet){
    std::vector<std::vector<unfoldtacpn::Colored::ColorExpression_ptr>> returnSet;
    for(auto set : rightSet){
        for(auto expr2 : leftSet){
            set.push_back(expr2);
            returnSet.push_back(set);
        }
    }
    return returnSet;
}

void PNMLParser::parseElement(rapidxml::xml_node<>* element) {

    for (auto it = element->first_node(); it; it = it->next_sibling()) {
        if (strcmp(it->name(), "place") == 0) {
            parsePlace(it);
        } else if (strcmp(it->name(),"transition") == 0) {
            parseTransition(it);
        } else if ( strcmp(it->name(),"arc") == 0 ||
                    strcmp(it->name(), "inputArc") == 0 ||
                    strcmp(it->name(), "outputArc") == 0) {
            parseArc(it);
        } else if (strcmp(it->name(),"transportArc") == 0) {
            parseTransportArc(it);
        } else if (strcmp(it->name(),"inhibitorArc") == 0) {
            parseArc(it, true);
        } else if (strcmp(it->name(), "variable") == 0) {
            std::cerr << "variable not supported" << std::endl;
            exit(ErrorCode);
        }
        else
        {
            parseElement(it);
        }
    }
}

std::pair<string, std::vector<const Colored::Color*>> PNMLParser::parseTimeConstraint(rapidxml::xml_node<> *element) { // parses the inscription and colors belonging to either an interval or invariant and returns it.
    string colorTypeName;
    string inscription;
    std::vector<const Colored::Color*> colors;
    for (auto i = element->first_node(); i; i = i->next_sibling()) {
        if (strcmp(i->name(), "colortype") == 0) {
            colorTypeName = i->first_attribute("name")->value();

            if (colorTypes.find(colorTypeName) == colorTypes.end()) {
                cerr << "The color type " << colorTypeName << " does not exist" << std::endl;
                exit(ErrorCode);
            }
            else {
                for (auto it = i->first_node(); it; it = it->next_sibling()) {
                    if (strcmp(it->name(), "color") == 0) {
                        Colored::Color* test = new Colored::Color(colorTypes[colorTypeName], 0, it->first_attribute("value")->value());
                        colors.push_back(test);

                    }
                    else {
                        std::cerr << "the colortype to the place element " << element->first_attribute("id")->value() << " does not have or should only have colors" << std::endl;
                        exit(ErrorCode);
                    }
                }
            }


        } else if (strcmp(i->name(), "inscription") == 0) {
            inscription = i->first_attribute("inscription")->value();

        }
    }
    return std::make_pair(inscription, colors);
}

void PNMLParser::parsePlace(rapidxml::xml_node<>* element) {
    double x = 0, y = 0;
    string starInvariant;
    string id(element->first_attribute("id")->value());

    auto initial = element->first_attribute("initialMarking");
    long long initialMarking = 0;
    unfoldtacpn::Colored::Multiset hlinitialMarking;
    unfoldtacpn::Colored::ColorType* type = nullptr;
    std::vector<unfoldtacpn::Colored::TimeInvariant> timeInvariants;
    if(initial)
         initialMarking = atoll(initial->value());

    auto posX = element->first_attribute("positionX");
    if (posX != nullptr){
         x = atoll(posX->value());
    }
    auto posY = element->first_attribute("positionY");
    if ( posY != nullptr){
        y = atoll(posY->value());
    }
    auto invariant = element->first_attribute("invariant");
    if(invariant != nullptr){
        starInvariant = invariant->value();
    }


    std::vector<const Colored::Color*> colors;
    colors.push_back(new Colored::Color());
    timeInvariants.push_back(Colored::TimeInvariant::createFor(starInvariant, colors, constantValues));

    for (auto it = element->first_node(); it; it = it->next_sibling()) {
        // name element is ignored
        if (strcmp(it->name(), "colorinvariant") == 0) {
            std::pair<string, std::vector<const Colored::Color*>> pair = parseTimeConstraint(it);
            timeInvariants.push_back(Colored::TimeInvariant::createFor(pair.first, pair.second, constantValues));
        } else if (strcmp(it->name(),"hlinitialMarking") == 0) {
            std::unordered_map<std::string, const unfoldtacpn::Colored::Color*> binding;
            unfoldtacpn::Colored::ExpressionContext context {binding, colorTypes};
            hlinitialMarking = parseArcExpression(it->first_node("structure"))->eval(context);
        } else if (strcmp(it->name(), "type") == 0) {
            type = parseUserSort(it);
        }
    }

    if(initialMarking > std::numeric_limits<int>::max())
    {
        std::cerr << "Number of tokens in " << id << " exceeded " << std::numeric_limits<int>::max() << std::endl;
        exit(ErrorCode);
    }
    //Create place
    if (!type) {
        type = Colored::DotConstant::dotConstant(nullptr)->getColorType();
    }
    builder->addPlace(id, std::move(hlinitialMarking), type, timeInvariants, x, y);
    //Map id to name
    NodeName nn;
    nn.id = id;
    nn.isPlace = true;
    id2name[id] = nn;
}

void PNMLParser::parseArc(rapidxml::xml_node<>* element, bool inhibitor) {
    string source = element->first_attribute("source")->value(),
            target = element->first_attribute("target")->value();
    int weight = 1;
    auto type = element->first_attribute("type");
    bool transport = false;
    string starInterval;
    bool normal = false;
    std::string transportID = "0";
    std::vector<Colored::TimeInterval> timeIntervals;
    if((type && strcmp(type->value(), "timed") == 0) || (element && strcmp(element->name(), "timedArc") == 0))
    {
        //TODO:: parse standard interval
    }
    else if((type && strcmp(type->value(), "inhibitor") == 0) || (element && strcmp(element->name(), "inhibitorArc") == 0) || (type && strcmp(type->value(), "tapnInhibitor") == 0))
    {
        inhibitor = true;
    }
    else if((type && strcmp(type->value(), "transport") == 0) || (element && strcmp(element->name(), "transportArc") == 0))
    {
        transportID = element->first_attribute("transportID")->value();
        transport = true;
    }
    else if ((type && strcmp(type->value(), "normal") == 0) || (element && (strcmp(element->name(), "inputArc") == 0 || strcmp(element->name(), "outputArc") == 0))) {
        normal = true;
    }

    bool first = true;
    auto weightTag = element->first_attribute("weight");
    if(weightTag != nullptr){
        string text;
        weight = atoi(weightTag->value());
        assert(weight > 0);
    } else {
        for (auto it = element->first_node("inscription"); it; it = it->next_sibling("inscription")) {
            string text;
            parseValue(it, text);
            weight = atoi(text.c_str());
            /*
            if(std::find_if(text.begin(), text.end(), [](char c) { return !std::isdigit(c) && !std::isblank(c); }) != text.end())
            {
                std::cerr << "ERROR: Found non-integer-text in inscription-tag (weight) on arc from " << source << " to " << target << " with value \"" << text << "\". An integer was expected." << std::endl;
                exit(ErrorCode);
            }*/
            assert(weight > 0);
            if(!first)
            {
                std::cerr << "ERROR: Multiple inscription tags in xml of a arc from " << source << " to " << target << "." << std::endl;
                exit(ErrorCode);
            }
            first = false;
        }
    }

    unfoldtacpn::Colored::ArcExpression_ptr expr;
    first = true;
    for (auto it = element->first_node("hlinscription"); it; it = it->next_sibling("hlinscription")) {
        expr = parseArcExpression(it->first_node("structure"));
        if(!first)
        {
            std::cerr << "ERROR: Multiple hlinscription tags in xml of a arc from " << source << " to " << target << "." << std::endl;
            exit(ErrorCode);
        }
        first = false;
    }
    if(!normal && !inhibitor) {
        starInterval = element->first_attribute("inscription")->value();
        std::vector<const Colored::Color*> colors;
        colors.push_back(new Colored::Color());
        timeIntervals.push_back(Colored::TimeInterval::createFor(starInterval, colors, constantValues));
        for (auto it = element->first_node("colorinterval"); it; it = it->next_sibling("colorinterval")) {
            std::pair<string, std::vector<const Colored::Color*>> pair = parseTimeConstraint(it);
            timeIntervals.push_back(Colored::TimeInterval::createFor(pair.first, pair.second, constantValues));
        }
    }

    if (!normal && !inhibitor)
        assert(expr != nullptr);
    Arc arc;
    arc.source = source;
    arc.target = target;
    arc.weight = weight;
    arc.inhib = inhibitor;
    arc.transport = transport;
    arc.normal = normal;
    arc.transportID = transportID;
    if(!inhibitor)
        arc.expr = expr;
    if(!normal && !inhibitor)
        arc.interval = timeIntervals;
    assert(weight > 0);

    if(weight != 0)
    {
        arcs.push_back(arc);
    }
    else
    {
        std::cerr << "ERROR: Arc from " << source << " to " << target << " has non-sensible weight 0." << std::endl;
        exit(ErrorCode);
    }
}

void PNMLParser::parseTransportArc(rapidxml::xml_node<>* element){
    string source	= element->first_attribute("source")->value(),
           transiton	= element->first_attribute("transition")->value(),
           target	= element->first_attribute("target")->value();
    int weight = 1;

    for(auto it = element->first_node("inscription"); it; it = it->next_sibling("inscription")){
        string text;
        parseValue(it, text);
        weight = atoi(text.c_str());
    }

    Arc inArc;
    inArc.source = source;
    inArc.target = transiton;
    inArc.weight = weight;
    arcs.push_back(inArc);

    Arc outArc;
    outArc.source = transiton;
    outArc.target = target;
    outArc.weight = weight;
    arcs.push_back(outArc);
}

void PNMLParser::parseTransition(rapidxml::xml_node<>* element) {
    Transition t;
    t.x = 0;
    t.y = 0;
    t.id = element->first_attribute("id")->value();
    t.expr = nullptr;
    t.urgent = false;


    auto posX = element->first_attribute("positionX");
    if (posX != nullptr){
         t.x = atoll(posX->value());
    }
    auto posY = element->first_attribute("positionY");
    if ( posY != nullptr){
        t.y = atoll(posY->value());
    }
    auto urgent = element->first_attribute("urgent");
    if (urgent != nullptr) {
        t.urgent = stringToBool(urgent->value());
    }


    for (auto it = element->first_node(); it; it = it->next_sibling()) {
        // name element is ignored
        if (strcmp(it->name(), "graphics") == 0) {
            parsePosition(it, t.x, t.y);
        } else if (strcmp(it->name(), "condition") == 0) {
            t.expr = parseGuardExpression(it->first_node("structure"));
        } else if (strcmp(it->name(), "conditions") == 0) {
            std::cerr << "conditions not supported" << std::endl;
            exit(ErrorCode);
        } else if (strcmp(it->name(), "assignments") == 0) {
            std::cerr << "assignments not supported" << std::endl;
            exit(ErrorCode);
        }
    }
    //Add transition to list
    transitions.push_back(t);
    //Map id to name
    NodeName nn;
    nn.id = t.id;
    nn.isPlace = false;
    id2name[t.id] = nn;
}

void PNMLParser::parseValue(rapidxml::xml_node<>* element, string& text) {
    for (auto it = element->first_node(); it; it = it->next_sibling()) {
        if (strcmp(it->name(), "value") == 0 || strcmp(it->name(), "text") == 0) {
            text = it->value();
        } else
            parseValue(it, text);
    }
}

uint32_t PNMLParser::parseNumberConstant(rapidxml::xml_node<>* element) {
    if (strcmp(element->name(), "numberconstant") == 0) {
        auto value = element->first_attribute("value")->value();
        return (uint32_t)atoll(value);
    } else if (strcmp(element->name(), "subterm") == 0) {
        return parseNumberConstant(element->first_node());
    }
    return 0;
}

void PNMLParser::parsePosition(rapidxml::xml_node<>* element, double& x, double& y) {
    for (auto it = element->first_node(); it; it = it->first_node()) {
        if (strcmp(it->name(), "position") == 0) {
            x = atof(it->first_attribute("x")->value());
            y = atof(it->first_attribute("y")->value());
        } else
        {
            parsePosition(it, x, y);
        }
    }
}

const unfoldtacpn::Colored::Color* PNMLParser::findColor(const char* name) const {
    for (const auto& elem : colorTypes) {
        auto col = (*elem.second)[name];
        if (col)
            return col;
    }
    printf("Could not find color: %s\nCANNOT_COMPUTE\n", name);
    exit(ErrorCode);
}

