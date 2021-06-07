/*
 * File:   ColoredPetriNetBuilder.cpp
 * Author: Klostergaard
 * 
 * Created on 17. februar 2018, 16:25
 */

#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"
#include <chrono>
#include <tuple>
using std::get;

namespace PetriEngine {
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

    void ColoredPetriNetBuilder::addPlace(const std::string& name, int tokens, double x, double y) {
        
        if (!_isColored) {
            _ptBuilder.addPlace(name, tokens, x, y);
        }
    }

    void ColoredPetriNetBuilder::addPlace(const std::string& name, Colored::ColorType* type, Colored::Multiset&& tokens, double x, double y) {
        if(_placenames.count(name) == 0)
        {
            uint32_t next = _placenames.size();
            _places.emplace_back(Colored::Place {name, type, tokens});
            _placenames[name] = next;
            
            _invariantStrings[name] = "< inf";
            _placelocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void ColoredPetriNetBuilder::addPlace(const std::string &name,
                                          PetriEngine::Colored::Multiset &&tokens,
                                          Colored::ColorType* type,
                                          std::vector<PetriEngine::Colored::TimeInvariant> &invariant, double x,
                                          double y) {
        if(_placenames.count(name) == 0){
            uint32_t next = _placenames.size();
            _places.emplace_back(Colored::Place{name, type, tokens, invariant});
            _placenames[name] = next;
            
            _invariantStrings[name] = invariant.size() ? invariant[0].toString(): "< inf";
            _placelocations.push_back(std::tuple<double, double>(x,y));
        }
    }

    void ColoredPetriNetBuilder::addTransition(const std::string& name, double x, double y) {
        if (!_isColored) {
            _ptBuilder.addTransition(name, x, y);
        }
    }

    void ColoredPetriNetBuilder::addTransition(const std::string& name, const Colored::GuardExpression_ptr& guard, double x, double y) {
        if(_transitionnames.count(name) == 0)
        {
            uint32_t next = _transitionnames.size();
            _transitions.emplace_back(Colored::Transition {name, guard});
            _transitionnames[name] = next;
            _transitionlocations.push_back(std::tuple<double, double>(x,y));
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

    void ColoredPetriNetBuilder::addInputArc(const std::string& place, const std::string& transition, bool inhibitor, int weight) {
        if (!_isColored) {
            _ptBuilder.addInputArc(place, transition, inhibitor, weight);
        }
    }

    void ColoredPetriNetBuilder::addInputArc(const std::string& place, const std::string& transition, const Colored::ArcExpression_ptr& expr, bool inhibitor, int weight) {
        addArc(place, transition, expr, true, inhibitor, weight);
    }

    void ColoredPetriNetBuilder::addInputArc(const std::string &place,
                                             const std::string &transition,
                                             bool inhibitor, bool transport,
                                             const Colored::ArcExpression_ptr& expr,
                                             std::vector<PetriEngine::Colored::TimeInterval> &interval, int weight, std::string transportID) {
        addTimedArc(place,transition,expr,interval,inhibitor, transport, true, weight, transportID);

    }

    void ColoredPetriNetBuilder::addOutputArc(const std::string& transition, const std::string& place, int weight, bool transport, std::string transportID) {
        if (!_isColored) {
            _ptBuilder.addOutputArc(transition, place, weight, transport, transportID);
        }
    }

    void ColoredPetriNetBuilder::addOutputArc(const std::string& transition, const std::string& place, const Colored::ArcExpression_ptr& expr) {
        addArc(place, transition, expr, false, false, 1);
    }

    void ColoredPetriNetBuilder::addOutputArc(const std::string& transition, const std::string& place, const Colored::ArcExpression_ptr& expr, bool transport, std::string transportID) {
        std::vector<PetriEngine::Colored::TimeInterval> intervals;
        addTimedArc(place, transition, expr, intervals, false, transport, false, 0, transportID);
    }

    void ColoredPetriNetBuilder::addArc(const std::string& place, const std::string& transition, const Colored::ArcExpression_ptr& expr, bool input, bool inhibitor, int weight) {
        if(_transitionnames.count(transition) == 0)
        {
            std::cout << "Transition '" << transition << "' not found. Adding it." << std::endl;
            addTransition(transition,0.0,0.0);
        }
        if(_placenames.count(place) == 0)
        {
            std::cout << "Place '" << place << "' not found. Adding it." << std::endl;
            addPlace(place,0,0,0);
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
        arc.expr = std::move(expr);
        arc.input = input;
        _places[p].inhibitor |= inhibitor;
        if(inhibitor){
            inhibitorArcs.push_back(std::move(arc));
        } else {
            _transitions[t].arcs.push_back(std::move(arc));
        }
        
        
    }

    void ColoredPetriNetBuilder::addTimedArc(const std::string &place, const std::string &transition,
                                             const PetriEngine::Colored::ArcExpression_ptr &expr,
                                             std::vector<PetriEngine::Colored::TimeInterval> &interval, bool inhibitor, bool transport,
                                             bool input, int weight, std::string transportID) {
        if(_transitionnames.count(transition) == 0)
        {
            std::cout << "Transition '" << transition << "' not found. Adding it." << std::endl;
            addTransition(transition,0.0,0.0);
        }
        if(_placenames.count(place) == 0)
        {
            std::cout << "Place '" << place << "' not found. Adding it." << std::endl;
            addPlace(place,0,0,0);
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
        arc.input = input;
        arc.weight = weight;
        arc.interval = interval;
        arc.inhibitor = inhibitor;
        arc.transport = transport;
        arc.transportID = transportID;
        // _transitions[t].inhibitor |= inhibitor;
         _places[p].inhibitor |= inhibitor;
        if(inhibitor){
            inhibitorArcs.push_back(std::move(arc));
        } else {
            _transitions[t].arcs.push_back(std::move(arc));
        }
        
    }

    void ColoredPetriNetBuilder::addTransportArc(const std::string &source, const std::string &transition,
                                                 const std::string &destination,
                                                 const PetriEngine::Colored::ArcExpression_ptr &expr,
                                                 std::vector<PetriEngine::Colored::TimeInterval> &interval) {
        if(_transitionnames.count(transition) == 0)
        {
            std::cout << "Transition '" << transition << "' not found. Adding it." << std::endl;
            addTransition(transition,0.0,0.0);
        }
        if(_placenames.count(source) == 0)
        {
            std::cout << "Place '" << source << "' not found. Adding it." << std::endl;
            addPlace(source,0,0,0);
        }
        if(_placenames.count(destination) == 0)
        {
            std::cout << "Place '" << destination << "' not found. Adding it." << std::endl;
            addPlace(destination,0,0,0);
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
    }

    void ColoredPetriNetBuilder::addColorType(const std::string& id, Colored::ColorType* type) {
        _colors[id] = type;
    }

    void ColoredPetriNetBuilder::sort() {

    }

    PetriNetBuilder& ColoredPetriNetBuilder::unfold() {
        if (_stripped) assert(false);
        if (_isColored && !_unfolded) {
            auto start = std::chrono::high_resolution_clock::now();
            for (auto& place : _places) {
                unfoldPlace(place);
            }

            for (auto& transition : _transitions) {
                unfoldTransition(transition);
            }

            _unfolded = true;
            auto end = std::chrono::high_resolution_clock::now();
            _time = (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count())*0.000001;
        }
        if (isTimed()) {
            _ptBuilder.setTimed(true);
        }

        return _ptBuilder;
    }

    void ColoredPetriNetBuilder::unfoldPlace(Colored::Place& place) {
        double xBuffer = 0;
        double yBuffer = 0;
        uint32_t index = _placenames[place.name];
        auto placePos = _placelocations[index];
        for (size_t i = 0; i < place.type->size(); ++i) {
            double x = xBuffer + get<0>(placePos);
            double y = yBuffer + get<1>(placePos);
            std::string name = place.name + "Sub" + std::to_string(i);
            const Colored::Color* color = &place.type->operator[](i);
            Colored::TimeInvariant invariant = getTimeInvariantForPlace(place.invariants, color); //TODO:: this does not take the correct time invariant
            _ptBuilder.addPlace(name, place.marking[color], invariant, x, y);
            _invariantStrings[name] = invariant.toString();

            _ptplacenames[place.name][color->getId()] = std::move(name);
            ++_nptplaces;
            xBuffer += 50;
            yBuffer += 0;
            
        }
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
                std::vector<const Colored::Color*> colors = color.getTuple();
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

    void ColoredPetriNetBuilder::unfoldTransition(Colored::Transition& transition) {
        BindingGenerator gen(transition, _colors);
        double buffer = 0;
        uint32_t transitionId = _transitionnames[transition.name];
        auto transitionPos = _transitionlocations[transitionId];
        size_t i = 0;
        for (auto& b : gen) {
            std::string name = transition.name + "Sub" + std::to_string(i++);
            if (isTimed()) {
                _ptBuilder.addTransition(name, transition.urgent, get<0>(transitionPos) + buffer, get<1>(transitionPos));
            } else {
                _ptBuilder.addTransition(name, get<0>(transitionPos) + buffer, get<1>(transitionPos));
            }
            _pttransitionnames[transition.name].push_back(name);
            ++_npttransitions;
            for (auto& arc : transition.arcs) {
                unfoldArc(arc, b, name);
            }
            unfoldInhibitorArc(transition.name, name);
            buffer += 250;
        }
    }

    void ColoredPetriNetBuilder::unfoldInhibitorArc(std::string &oldname, std::string &newname) {
        for (uint32_t i = 0; i < inhibitorArcs.size(); ++i) {
            if (_transitions[inhibitorArcs[i].transition].name.compare(oldname) == 0) {
                Colored::Arc inhibArc = inhibitorArcs[i];
                std::string placeName = _sumPlacesNames[inhibArc.place];

                if(placeName.empty()){
                    Colored::Color color;
                    const PetriEngine::Colored::Place& place = _places[inhibArc.place]; 
                    placeName = place.name + "Sum";
                    _ptBuilder.addPlace(placeName, place.marking.size(), Colored::TimeInvariant(color), 0, 0);
                    _invariantStrings[placeName] = Colored::TimeInvariant(color).toString();
                    //_ptplacenames[place.name][color.getId()] = std::move(placeName);
                    if(_ptplacenames.count(place.name) <= 0){
                        _ptplacenames[place.name][0] = placeName;
                    }
                    _sumPlacesNames[inhibArc.place] = std::move(placeName);
                }
                _ptBuilder.addInputArc(placeName, newname, true, inhibArc.weight);
            }
        }
    }

    void ColoredPetriNetBuilder::unfoldArc(Colored::Arc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& tName) {
        Colored::ExpressionContext context {binding, _colors};
        const PetriEngine::Colored::Place& place = _places[arc.place];
        auto ms = arc.expr->eval(context);
        int shadowWeight = 0;
        std::vector<Colored::TimeInterval>& timeIntervals = arc.interval;
        for (const auto& color : ms) {
            if (color.second == 0) {
                continue;
            }
            shadowWeight += color.second;
            const std::string& pName = _ptplacenames[place.name][color.first->getId()];
            if(isTimed()){
                if (!arc.input) {
                    _ptBuilder.addOutputArc(tName, pName, color.second, arc.transport, arc.transportID);
                } else {
                    Colored::TimeInterval timeInterval = getTimeIntervalForArc(timeIntervals, color.first);
                    _ptBuilder.addInputArc(pName, tName, arc.inhibitor, arc.transport, color.second, timeInterval, arc.transportID);
                }
            } else {
                if (!arc.input) {
                    _ptBuilder.addOutputArc(tName, pName, color.second);
                } else {
                    _ptBuilder.addInputArc(pName, tName, arc.inhibitor, color.second);
                }
            }
             
            ++_nptarcs;
        }

        if(place.inhibitor){
            Colored::Color color;
            const std::string &sumPlaceName = _sumPlacesNames[arc.place];
            if(sumPlaceName.empty()){
                const std::string &newSumPlaceName = place.name + "Sum";
                _ptBuilder.addPlace(newSumPlaceName, place.marking.size(), Colored::TimeInvariant(color), 0, 0);
                _invariantStrings[newSumPlaceName] = Colored::TimeInvariant(color).toString();
                //_ptplacenames[place.name][color.getId()] = std::move(placeName);
                _sumPlacesNames[arc.place] = std::move(newSumPlaceName);
            }
            

            if(shadowWeight > 0) {
                if (!arc.input) {
                    _ptBuilder.addOutputArc(tName, sumPlaceName, shadowWeight);
                } else {
                    Colored::TimeInterval timeInterval(color);
                    _ptBuilder.addInputArc(sumPlaceName, tName, false, false, shadowWeight, timeInterval, arc.transportID);
                }
                ++_nptarcs;
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

    PetriNetBuilder& ColoredPetriNetBuilder::stripColors() {
        if (_unfolded) assert(false);
        if (_isColored && !_stripped) {
            for (auto& place : _places) {
                if (!isTimed())
                    _ptBuilder.addPlace(place.name, place.marking.size(), 0.0, 0.0);
            }

            for (auto& transition : _transitions) {
                _ptBuilder.addTransition(transition.name, 0.0, 0.0);
                for (auto& arc : transition.arcs) {
                    try {
                        if (arc.input) {
                            _ptBuilder.addInputArc(_places[arc.place].name, _transitions[arc.transition].name, false,
                                                   arc.expr->weight());
                        } else {
                            _ptBuilder.addOutputArc(_transitions[arc.transition].name, _places[arc.place].name,
                                                    arc.expr->weight(), arc.transport, arc.transportID);
                        }
                    } catch (Colored::WeightException& e) {
                        std::cerr << "Exception on arc: " << arcToString(arc) << std::endl;
                        std::cerr << "In expression: " << arc.expr->toString() << std::endl;
                        std::cerr << e.what() << std::endl;
                        exit(ErrorCode);
                    }
                }
            }

            _stripped = true;
            _isColored = false;
        }

        return _ptBuilder;
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

