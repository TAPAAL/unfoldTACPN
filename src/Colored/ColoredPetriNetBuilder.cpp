/*
 * File:   ColoredPetriNetBuilder.cpp
 * Author: Klostergaard
 *
 * Created on 17. februar 2018, 16:25
 */

#include <chrono>
#include <tuple>

#include "Colored/ColoredPetriNetBuilder.h"
#include "PetriParse/PNMLParser.h"

namespace unfoldtacpn {
    ColoredPetriNetBuilder::ColoredPetriNetBuilder() {
    }

    ColoredPetriNetBuilder::ColoredPetriNetBuilder(const ColoredPetriNetBuilder& orig)
    : _placenames(orig._placenames), _transitionnames(orig._transitionnames),
       _placelocations(orig._placelocations), _transitionlocations(orig._transitionlocations),
       _transitions(orig._transitions), _places(orig._places)
    {

    }

    ColoredPetriNetBuilder::~ColoredPetriNetBuilder() {
    }

    void ColoredPetriNetBuilder::parseNet(std::istream& stream) {
        PNMLParser parser;
        parser.parse(stream, this);
    }

    void ColoredPetriNetBuilder::addPlace(const std::string &name,
                                          unfoldtacpn::Colored::Multiset &&tokens,
                                          Colored::ColorType* type,
                                          std::vector<unfoldtacpn::Colored::TimeInvariant> &invariant, double x,
                                          double y) {
        if(_placenames.count(name) == 0){
            uint32_t next = _placenames.size();
            _places.emplace_back(Colored::Place{name, type, tokens, invariant});
            _placenames[name] = next;

            _invariantStrings[name] = invariant.size() ? invariant[0].toString(): "< inf";
            _placelocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void ColoredPetriNetBuilder::addTransition(const std::string& name,
            const Colored::GuardExpression_ptr& guard,
            bool urgent,
            double x,
            double y) {
        if (_transitionnames.count(name) == 0) {
            uint32_t next = _transitionnames.size();
            _transitions.emplace_back(Colored::Transition {name, guard, urgent});
            _transitionnames[name] = next;
            _transitionlocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void ColoredPetriNetBuilder::addArc(const std::string& source, const std::string& target,
                    int weight, bool inhibitor, const unfoldtacpn::Colored::ArcExpression_ptr &expr,
                                             const std::vector<unfoldtacpn::Colored::TimeInterval>& intervals) {
        auto& transition = _transitionnames.count(source) == 0 ? target : source;
        auto& place = _transitionnames.count(source) == 0 ? source : target;

        if(_transitionnames.count(transition) == 0)
        {
            std::cout << "Transition '" << transition << "' not found." << std::endl;
            std::exit(ErrorCode);
        }
        if(_placenames.count(place) == 0)
        {
            std::cout << "Place '" << place << "' not found." << std::endl;
            std::exit(ErrorCode);
        }
        uint32_t p = _placenames[place];
        uint32_t t = _transitionnames[transition];

        assert(t < _transitions.size());
        assert(p < _places.size());

        Colored::Arc arc;
        arc.place = p;
        arc.transition = t;
        if(!inhibitor)
            assert(expr != nullptr);
        arc.expr = expr;
        arc.input = (&source) == (&place);
        arc.weight = weight;
        arc.interval = intervals;
        arc.inhibitor = inhibitor;
        if(inhibitor){
            _inhibitorArcs.push_back(std::move(arc));
        } else {
            _transitions[t].arcs.push_back(std::move(arc));
        }
    }

    void ColoredPetriNetBuilder::addTransportArc(const std::string &source, const std::string &transition,
                                                 const std::string &destination, int weight,
                                                 const unfoldtacpn::Colored::ArcExpression_ptr &expr,
                                                 std::vector<unfoldtacpn::Colored::TimeInterval> &interval) {
        if(_transitionnames.count(transition) == 0)
        {
            std::cout << "Transition '" << transition << "' not found." << std::endl;
            std::exit(ErrorCode);
        }
        if(_placenames.count(source) == 0)
        {
            std::cout << "Place '" << source << "' not found." << std::endl;
            std::exit(ErrorCode);
        }
        if(_placenames.count(destination) == 0)
        {
            std::cout << "Place '" << destination << "' not found." << std::endl;
            std::exit(ErrorCode);
        }
        uint32_t s = _placenames[source];
        uint32_t t = _transitionnames[transition];

        assert(t < _transitions.size());
        assert(s < _places.size());

        Colored::TransportArc transportArc;
        transportArc.source = s;
        transportArc.transition = t;
        assert(expr != nullptr);
        transportArc.expr = expr;
        transportArc.interval = interval;
        transportArc.weight = weight;
    }

    void ColoredPetriNetBuilder::addColorType(const std::string& id, Colored::ColorType* type) {
        _colors[id] = type;
    }

    void ColoredPetriNetBuilder::unfold(TAPNBuilderInterface& builder) {
        clear();
        auto start = std::chrono::high_resolution_clock::now();
        for (auto& place : _places) {
            unfoldPlace(builder, place);
        }

        for (auto& transition : _transitions) {
            unfoldTransition(builder, transition);
        }

        auto end = std::chrono::high_resolution_clock::now();
        _time = (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count())*0.000001;
    }

    void ColoredPetriNetBuilder::unfoldPlace(TAPNBuilderInterface& builder, Colored::Place& place) {
        double xBuffer = 0;
        double yBuffer = 0;
        uint32_t index = _placenames[place.name];
        auto placePos = _placelocations[index];
        for (size_t i = 0; i < place.type->size(); ++i) {
            double x = xBuffer + std::get<0>(placePos);
            double y = yBuffer + std::get<1>(placePos);
            std::string name = place.name + "Sub" + std::to_string(i);
            const Colored::Color* color = &place.type->operator[](i);
            Colored::TimeInvariant invariant = getTimeInvariantForPlace(place.invariants, color); //TODO:: this does not take the correct time invariant
            builder.addPlace(name, place.marking[color], invariant.isBoundStrict(), invariant.getBound(), x, y);
            _invariantStrings[name] = invariant.toString();

            _ptplacenames[place.name][color->getId()] = std::move(name);
            xBuffer += 50;
            yBuffer += 0;

        }
        double x = xBuffer + std::get<0>(placePos);
        double y = yBuffer + std::get<1>(placePos);
        Colored::Color color;
        std::string placeName = place.name + "Sum";
        builder.addPlace(placeName, place.marking.size(), false, std::numeric_limits<int>::max(), x, y);
        _invariantStrings[placeName] = Colored::TimeInvariant(color).toString();
        _shadowPlacesNames[place.name] = std::move(placeName);
    }

    Colored::TimeInvariant ColoredPetriNetBuilder::getTimeInvariantForPlace(std::vector< Colored::TimeInvariant> TimeInvariants, const Colored::Color* color) {
        for (Colored::TimeInvariant element : TimeInvariants) {
            if (!element.getColor().isTuple()) {
                if (element.getColor().getId() == color->getId()) {
                    return element;
                }
            } else {
                bool matches = true;
                Colored::Color color = element.getColor();
                const auto& colors = color.getTuple();
                if (colors.size() == color.getTuple().size()) {
                    for(uint32_t i = 0;i < colors.size(); i++) {
                        if (colors[i]->getId() != color.getTuple()[i]->getId())
                            matches = false;
                    }
                    if (matches)
                        return element;
                }
            }
        }
        for (uint32_t j = 0; j < TimeInvariants.size(); ++j) {
            if (TimeInvariants[j].getColor().getColorName() == "*") {
                return TimeInvariants[j];
            }
        }
        exit(ErrorCode);
    }

    void ColoredPetriNetBuilder::unfoldTransition(TAPNBuilderInterface& builder, Colored::Transition& transition) {
        BindingGenerator gen(transition, _colors);
        double buffer = 0;
        uint32_t transitionId = _transitionnames[transition.name];
        auto transitionPos = _transitionlocations[transitionId];
        size_t i = 0;
        for (auto& b : gen) {
            std::string name = transition.name + "Sub" + std::to_string(i++);
            builder.addTransition(name, transition.urgent, std::get<0>(transitionPos) + buffer, std::get<1>(transitionPos));
            _pttransitionnames[transition.name].push_back(name);
            for (auto& arc : transition.arcs) {
                unfoldArc(builder, arc, b, name);
            }
            unfoldInhibitorArc(builder, transition.name, name);
            buffer += 250;
        }
    }

    void ColoredPetriNetBuilder::unfoldInhibitorArc(TAPNBuilderInterface& builder, std::string &oldname, std::string &newname) {
        for (uint32_t i = 0; i < _inhibitorArcs.size(); ++i) {
            if (_transitions[_inhibitorArcs[i].transition].name.compare(oldname) == 0) {
                Colored::Arc inhibArc = _inhibitorArcs[i];
                std::string place = _shadowPlacesNames[_places[inhibArc.place].name];
                builder.addInputArc(place, newname, inhibArc.inhibitor, inhibArc.weight, false, false, 0, 0);
            }
        }
    }

    void ColoredPetriNetBuilder::unfoldArc(TAPNBuilderInterface& builder, Colored::Arc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& tName) {
        Colored::ExpressionContext context {binding, _colors};
        auto ms = arc.expr->eval(context);
        int shadowWeight = 0;
        std::vector<Colored::TimeInterval>& timeIntervals = arc.interval;
        for (const auto& color : ms) {
            if (color.second == 0) {
                continue;
            }
            shadowWeight += color.second;
            const std::string& pName = _ptplacenames[_places[arc.place].name][color.first->getId()];
            if (!arc.input) {
                builder.addOutputArc(tName, pName, color.second);
            } else {
                Colored::TimeInterval timeInterval = getTimeIntervalForArc(timeIntervals, color.first);
                builder.addInputArc(pName, tName, arc.inhibitor, color.second,
                    timeInterval.isLowerBoundStrict(), timeInterval.isUpperBoundStrict(),
                    timeInterval.getLowerBound(), timeInterval.getUpperBound());
            }
        }
        if(shadowWeight > 0) {
            std::string pName = _shadowPlacesNames[_places[arc.place].name];
            if (!arc.input) {
                builder.addOutputArc(tName, pName, shadowWeight);
            } else {
                Colored::Color color;
                Colored::TimeInterval timeInterval(color);
                builder.addInputArc(pName, tName, arc.inhibitor, shadowWeight,
                timeInterval.isLowerBoundStrict(), timeInterval.isUpperBoundStrict(),
                timeInterval.getLowerBound(), timeInterval.getUpperBound());
            }
        }
    }

    Colored::TimeInterval ColoredPetriNetBuilder::getTimeIntervalForArc(std::vector< Colored::TimeInterval> timeIntervals,const Colored::Color* color) {
        for (Colored::TimeInterval element : timeIntervals) {
            if (!element.getColor().isTuple()) {
                if (element.getColor().getId() == color->getId()) {
                    return element;
                }
            } else {
                bool matches = true;
                std::vector<const Colored::Color*> colors = element.getColor().getTuple();

                if (colors.size() == color->getTuple().size()) {
                    for(uint32_t i = 0; i < colors.size(); i++) {
                        if (colors[i]->getId() != color->getTuple()[i]->getId())
                            matches = false;
                    }
                    if (matches)
                        return element;
                }
            }
        }
        for (uint32_t j = 0; j < timeIntervals.size(); ++j) { // If there is no timeinterval for the specific color, we use the default * interval
            if (timeIntervals[j].getColor().getColorName() == "*") {
                return timeIntervals[j];
            }
        }
        std::cerr << "There is no matching time interval to an arc" << std::endl;;
        exit(ErrorCode);
    }

    std::string ColoredPetriNetBuilder::arcToString(Colored::Arc& arc) const {
        return !arc.input ? "(" + _transitions[arc.transition].name + ", " + _places[arc.place].name + ")" :
               "(" + _places[arc.place].name + ", " + _transitions[arc.transition].name + ")";
    }

    BindingGenerator::Iterator::Iterator(BindingGenerator* generator)
            : _generator(generator)
    {
    }

    bool BindingGenerator::Iterator::operator==(Iterator& other) {
        return _generator == other._generator;
    }

    bool BindingGenerator::Iterator::operator!=(Iterator& other) {
        return _generator != other._generator;
    }

    BindingGenerator::Iterator& BindingGenerator::Iterator::operator++() {
        _generator->nextBinding();
        if (_generator->isInitial()) _generator = nullptr;
        return *this;
    }

    const Colored::ExpressionContext::BindingMap BindingGenerator::Iterator::operator++(int) {
        auto prev = _generator->currentBinding();
        ++*this;
        return prev;
    }

    Colored::ExpressionContext::BindingMap& BindingGenerator::Iterator::operator*() {
        return _generator->currentBinding();
    }

    BindingGenerator::BindingGenerator(Colored::Transition& transition,
            ColoredPetriNetBuilder::ColorTypeMap& colorTypes)
        : _colorTypes(colorTypes)
    {
        _expr = transition.guard;
        std::set<Colored::Variable*> variables;
        if (_expr != nullptr) {
            _expr->getVariables(variables);
        }
        for (auto arc : transition.arcs) {
            assert(arc.expr != nullptr);
            arc.expr->getVariables(variables);
        }
        for (auto var : variables) {
            _bindings[var->name] = &var->colorType->operator[](0);
        }

        if (!eval())
            nextBinding();
    }

    bool BindingGenerator::eval() {
        if (_expr == nullptr)
            return true;

        Colored::ExpressionContext context {_bindings, _colorTypes};
        return _expr->eval(context);
    }

    Colored::ExpressionContext::BindingMap& BindingGenerator::nextBinding() {
        bool test = false;
        while (!test) {
            for (auto& _binding : _bindings) {
                _binding.second = &_binding.second->operator++();
                if (_binding.second->getId() != 0) {
                    break;
                }
            }

            if (isInitial())
                break;

            test = eval();
        }
        return _bindings;
    }

    Colored::ExpressionContext::BindingMap& BindingGenerator::currentBinding() {
        return _bindings;
    }

    bool BindingGenerator::isInitial() const {
        for (auto& b : _bindings) {
            if (b.second->getId() != 0) return false;
        }
        return true;
    }

    BindingGenerator::Iterator BindingGenerator::begin() {
        return {this};
    }

    BindingGenerator::Iterator BindingGenerator::end() {
        return {nullptr};
    }
}

