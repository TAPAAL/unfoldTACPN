/*
 * File:   ColoredPetriNetBuilder.h
 * Author: Klostergaard
 *
 * Created on 17. februar 2018, 16:25
 */

#ifndef COLOREDPETRINETBUILDER_H
#define COLOREDPETRINETBUILDER_H

#include <vector>
#include <unordered_map>

#include "ColoredNetStructures.h"
#include "../TAPNBuilderInterface.h"

namespace unfoldtacpn {
    class ColoredPetriNetBuilder {
    public:
        typedef std::unordered_map<std::string, Colored::ColorType*> ColorTypeMap;
        typedef std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>> PTPlaceMap;
        typedef std::unordered_map<std::string, std::vector<std::string>> PTTransitionMap;

    public:
        ColoredPetriNetBuilder();
        ColoredPetriNetBuilder(const ColoredPetriNetBuilder& orig);
        virtual ~ColoredPetriNetBuilder();
        void parseNet(std::istream& istream);

        void addPlace(const std::string& name,
                      Colored::Multiset&& tokens,
                      Colored::ColorType* type,
                      std::vector<Colored::TimeInvariant>& invariant,
                      double x = 0,
                      double y = 0);
        void addTransition(const std::string& name,
                           const Colored::GuardExpression_ptr& guard,
                           bool urgent,
                           double x,
                           double y);
        void addArc(const std::string& source, const std::string& target,
                    int weight, bool inhibitor, const unfoldtacpn::Colored::ArcExpression_ptr &expr,
                                             const std::vector<unfoldtacpn::Colored::TimeInterval>& intervals);

        void addColorType(const std::string& id,
                Colored::ColorType* type);

        void addTransportArc(const std::string& source,
                const std::string& transition,
                const std::string& destination,
                int weight,
                const Colored::ArcExpression_ptr& expr,
                std::vector<Colored::TimeInterval>& interval);

        double getUnfoldTime() const {
            return _time;
        }

        uint32_t getPlaceCount() const {
            return _places.size();
        }

        uint32_t getTransitionCount() const {
            return _transitions.size();
        }

        uint32_t getArcCount() const {
            uint32_t sum = 0;
            for (auto& t : _transitions) {
                sum += t.arcs.size();
            }
            return sum;
        }

        const PTPlaceMap& getUnfoldedPlaceNames() const {
            return _ptplacenames;
        }

        const PTTransitionMap& getUnfoldedTransitionNames() const {
            return _pttransitionnames;
        }

        void unfold(TAPNBuilderInterface& builder);
        void clear() { _shadowPlacesNames.clear(); _pttransitionnames.clear(); _ptplacenames.clear(); }
    private:
        std::unordered_map<std::string,uint32_t> _placenames;
        std::unordered_map<std::string,uint32_t> _transitionnames;
        PTPlaceMap _ptplacenames;
        std::unordered_map<std::string, std::string> _shadowPlacesNames;
        PTTransitionMap _pttransitionnames;
        std::vector< std::tuple<double, double> > _placelocations;
        std::vector< std::tuple<double, double> > _transitionlocations;

        std::vector<Colored::Transition> _transitions;
        std::vector<Colored::Place> _places;
        std::vector<Colored::Arc> _inhibitorArcs;
        ColorTypeMap _colors;
        double _time;

        std::string arcToString(Colored::Arc& arc) const ;

        Colored::TimeInterval getTimeIntervalForArc(std::vector< Colored::TimeInterval> timeIntervals,const Colored::Color* color);
        void unfoldPlace(TAPNBuilderInterface& builder, Colored::Place& place);
        Colored::TimeInvariant getTimeInvariantForPlace(std::vector< Colored::TimeInvariant> TimeInvariants, const Colored::Color* color);
        void unfoldTransition(TAPNBuilderInterface& builder, Colored::Transition& transition);
        void unfoldArc(TAPNBuilderInterface& builder, Colored::Arc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& name);
        void unfoldTransport(TAPNBuilderInterface& builder, Colored::TransportArc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& name);
        void unfoldInhibitorArc(TAPNBuilderInterface& builder, std::string &oldname, std::string &newname);
    };

    class BindingGenerator {
    public:
        class Iterator {
        private:
            BindingGenerator* _generator;

        public:
            Iterator(BindingGenerator* generator);

            bool operator==(Iterator& other);
            bool operator!=(Iterator& other);
            Iterator& operator++();
            const Colored::ExpressionContext::BindingMap operator++(int);
            Colored::ExpressionContext::BindingMap& operator*();
        };
    private:
        Colored::GuardExpression_ptr _expr;
        Colored::ExpressionContext::BindingMap _bindings;
        ColoredPetriNetBuilder::ColorTypeMap& _colorTypes;

        bool eval();

    public:
        BindingGenerator(Colored::Transition& transition,
                ColoredPetriNetBuilder::ColorTypeMap& colorTypes);

        Colored::ExpressionContext::BindingMap& nextBinding();
        Colored::ExpressionContext::BindingMap& currentBinding();
        bool isInitial() const;
        Iterator begin();
        Iterator end();
    };
}

#endif /* COLOREDPETRINETBUILDER_H */

