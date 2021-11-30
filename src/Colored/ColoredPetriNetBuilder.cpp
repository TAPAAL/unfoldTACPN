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
#include "errorcodes.h"

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
            if(type == nullptr)
                type = _colors["dot"];
            _places.emplace_back(Colored::Place{name, type, tokens, invariant});
            _placenames[name] = next;
            _placelocations.push_back(std::tuple<double, double>(x,y));
        }
        else
        {
            std::cerr << "ERROR: Adding place " << name << " twice!" << std::endl;
            std::exit(ErrorCode);
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

        if(weight <= 0)
        {
            std::cout << "Arc between " << source << " and " << target << " is 0" << std::endl;
            std::exit(ErrorCode);
        }

        if(inhibitor && &place != &source)
        {
            std::cout << "Source of inhibitor must be a place, but is " << source << std::endl;
            std::exit(ErrorCode);
        }

        // if we have an inhibitor, call intervals/guards must be zero-infinity.
        if(inhibitor && std::any_of(std::begin(intervals), std::end(intervals), [](auto& i){ return !i.isZeroInfinity();}))
        {
            std::cout << "Source inhibitors cannot have guards, between " << source << " and " << target << std::endl;
            std::exit(ErrorCode);
        }

        uint32_t p = _placenames[place];
        uint32_t t = _transitionnames[transition];

        assert(t < _transitions.size());
        assert(p < _places.size());

        Colored::Arc arc;
        arc.place = p;
        arc.transition = t;
        arc.expr = expr;
        if(arc.expr == nullptr)
        {
            std::vector<Colored::ColorExpression_ptr> colors{std::make_shared<Colored::DotConstantExpression>()};
            arc.expr = std::make_shared<Colored::NumberOfExpression>(
                                                std::move(colors), weight);
        }
        arc.input = (&source) == (&place);
        arc.weight = weight;
        arc.interval = intervals;
        arc.inhibitor = inhibitor;
        if(inhibitor){
            _inhibitorArcs[arc.transition].emplace_back(std::move(arc));
        } else {
            _transitions[t].arcs.emplace_back(std::move(arc));
        }
    }

    void ColoredPetriNetBuilder::addTransportArc(const std::string &source, const std::string &transition,
                                                 const std::string &destination, int weight,
                                                 const unfoldtacpn::Colored::ArcExpression_ptr &in_expr,
                                                 const unfoldtacpn::Colored::ArcExpression_ptr &out_expr,
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
        uint32_t d = _placenames[destination];

        assert(t < _transitions.size());
        assert(s < _places.size());

        Colored::TransportArc transportArc;
        transportArc.source = s;
        transportArc.transition = t;
        transportArc.destination = d;
        transportArc.in_expr = in_expr;
        transportArc.out_expr = out_expr;
        transportArc.interval = interval;
        transportArc.weight = weight;

        if(transportArc.in_expr == nullptr)
        {
            std::vector<Colored::ColorExpression_ptr> colors{std::make_shared<Colored::DotConstantExpression>()};
            transportArc.in_expr = std::make_shared<Colored::NumberOfExpression>(
                                                std::move(colors), weight);
        }
        if(transportArc.out_expr == nullptr)
        {
            std::vector<Colored::ColorExpression_ptr> colors{std::make_shared<Colored::DotConstantExpression>()};
            transportArc.out_expr = std::make_shared<Colored::NumberOfExpression>(
                                                std::move(colors), weight);
        }
        _transitions[t].transport.emplace_back(std::move(transportArc));
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
        size_t size = place.type == nullptr ? 1 : place.type->size();
        if(size != 1)
        {
            size_t i = 0;
            for (; i < place.type->size(); ++i) {
                double x = xBuffer + std::get<0>(placePos);
                double y = yBuffer + std::get<1>(placePos);
                std::string name = place.name + "Sub" + std::to_string(i);
                const Colored::Color* color = &place.type->operator[](i);
                Colored::TimeInvariant invariant = getTimeInvariantForPlace(place.invariants, color); //TODO:: this does not take the correct time invariant
                auto r = place.marking[color];
                builder.addPlace(name, r, invariant.isBoundStrict(), invariant.getBound(), x, y);

                _ptplacenames[place.name][color->getId()] = std::move(name);
                xBuffer += 50;
                yBuffer += 0;

            }
            double x = xBuffer + std::get<0>(placePos);
            double y = yBuffer + std::get<1>(placePos);
            std::string placeName = place.name + "Sum";
            builder.addPlace(placeName, place.marking.size(), true, std::numeric_limits<int>::max(), x, y);
            _shadowPlacesNames[place.name] = std::move(placeName);
        }
        else
        {
            _ptplacenames[place.name][0] = place.name;
            const unfoldtacpn::Colored::Color* color = &(*place.type)[0];
            Colored::TimeInvariant invariant = getTimeInvariantForPlace(place.invariants, color);
            builder.addPlace(place.name, place.marking.size(), invariant.isBoundStrict(), invariant.getBound(),
                std::get<0>(placePos), std::get<1>(placePos));
        }
    }

    Colored::TimeInvariant ColoredPetriNetBuilder::getTimeInvariantForPlace(std::vector< Colored::TimeInvariant> TimeInvariants, const Colored::Color* color) {
        if(color != nullptr)
        {
            for (Colored::TimeInvariant& element : TimeInvariants) {
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
                        {
                            return element;
                        }
                    }
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
            std::string name = transition.name;
            if(!gen.isInitial())
                name += "Sub" + std::to_string(i++);
            builder.addTransition(name, transition.urgent, std::get<0>(transitionPos) + buffer, std::get<1>(transitionPos));
            _pttransitionnames[transition.name].push_back(name);
            for (auto& arc : transition.arcs) {
                unfoldArc(builder, arc, b, name);
            }
            for (auto& transport : transition.transport)
            {
                unfoldTransport(builder, transport, b, name);
            }
            unfoldInhibitorArc(builder, transitionId, name);
            buffer += 250;
        }
    }

    void ColoredPetriNetBuilder::unfoldInhibitorArc(TAPNBuilderInterface& builder, uint32_t transition, const std::string &newname) {
        for (auto& inhibitor : _inhibitorArcs[transition]) {
            auto& place = findShadowName(inhibitor.place);
            if(place.size() != 0)
                builder.addInputArc(place, newname, true, inhibitor.weight, false, true, 0, std::numeric_limits<int>::max());
            else
                builder.addInputArc(_places[inhibitor.place].name, newname, true, inhibitor.weight, false, true, 0, std::numeric_limits<int>::max());
        }
    }

    void ColoredPetriNetBuilder::unfoldTransport(TAPNBuilderInterface& builder, Colored::TransportArc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& tName) {
        Colored::ExpressionContext context {binding, _colors};
        auto in_ms = arc.in_expr->eval(context);
        auto out_ms = arc.out_expr->eval(context);
        const auto& in_color = *in_ms.begin();
        const auto& out_color = *out_ms.begin();
        for(auto oc : in_ms)
        {
            if(*oc.first != *in_color.first)
            {
                std::cerr << "ERROR: Ill-formed transport-arc color" << std::endl;
                std::exit(ErrorCode);
            }
        }
        for(auto oc : out_ms)
        {
            if(*oc.first != *out_color.first)
            {
                std::cerr << "ERROR: Ill-formed transport-arc color" << std::endl;
                std::exit(ErrorCode);
            }
        }

        const std::string& inName = findPlaceName(arc.source, in_color.first);
        const std::string& outName = findPlaceName(arc.destination, out_color.first);
        {
            // add actual transport
            Colored::TimeInterval timeInterval = getTimeIntervalForArc(arc.interval, in_color.first);
            builder.addTransportArc(inName, tName, outName, in_color.second,
                    timeInterval.isLowerBoundStrict(), timeInterval.isUpperBoundStrict(),
                    timeInterval.getLowerBound(), timeInterval.getUpperBound());
        }
        {
            // add shadow
            auto& in_shadow = findShadowName(arc.source);
            if(in_shadow.size() != 0)
            {
                Colored::Color color;
                Colored::TimeInterval timeInterval(color);
                builder.addInputArc(in_shadow, tName, false, in_color.second,
                    timeInterval.isLowerBoundStrict(), timeInterval.isUpperBoundStrict(),
                    timeInterval.getLowerBound(), timeInterval.getUpperBound());
            }
            auto& out_shadow = findShadowName(arc.destination);
            if(out_shadow.size() > 0)
                builder.addOutputArc(out_shadow, findShadowName(arc.destination), out_color.second);
        }
    }

    const std::string empty_shadow;
    const std::string& ColoredPetriNetBuilder::findShadowName(const std::string& id)
    {
        auto it = _shadowPlacesNames.find(id);
        if(it == std::end(_shadowPlacesNames))
            return empty_shadow;
        else
            return it->second;
    }

    const std::string& ColoredPetriNetBuilder::findPlaceName(const std::string& place, const Colored::Color* color)
    {
        auto it = _ptplacenames.find(place);
        if(it == std::end(_ptplacenames))
            return place;
        else
            return it->second[color->getId()];
    }

    void ColoredPetriNetBuilder::unfoldArc(TAPNBuilderInterface& builder, Colored::Arc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& tName) {
        Colored::ExpressionContext context {binding, _colors};
        auto ms = arc.expr->eval(context);
        int shadowWeight = 0;
        std::vector<Colored::TimeInterval>& timeIntervals = arc.interval;
        bool is_singular = true;
        for (const auto& color : ms) {
            if (color.second == 0) {
                continue;
            }
            shadowWeight += color.second;
            is_singular &= color.first->getColorType()->size() == 1;
            auto& pName = findPlaceName(arc.place, color.first);
            if (!arc.input) {
                builder.addOutputArc(tName, pName, color.second);
            } else {
                Colored::TimeInterval timeInterval = getTimeIntervalForArc(timeIntervals, color.first);
                builder.addInputArc(pName, tName, arc.inhibitor, color.second,
                    timeInterval.isLowerBoundStrict(), timeInterval.isUpperBoundStrict(),
                    timeInterval.getLowerBound(), timeInterval.getUpperBound());
            }
        }
        if(shadowWeight > 0 && !is_singular) { // we only add shadow-places if we have a non-singleton color
            const std::string& pName = findShadowName(arc.place);
            assert(pName.size() > 0);
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

