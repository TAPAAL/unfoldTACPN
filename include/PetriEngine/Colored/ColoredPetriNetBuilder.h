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
#include "../AbstractPetriNetBuilder.h"
#include "../PetriNetBuilder.h"

namespace PetriEngine {
    class ColoredPetriNetBuilder : public AbstractPetriNetBuilder {
    public:
        typedef std::unordered_map<std::string, Colored::ColorType*> ColorTypeMap;
        typedef std::unordered_map<std::string, std::unordered_map<uint32_t , std::string>> PTPlaceMap;
        typedef std::unordered_map<std::string, std::vector<std::string>> PTTransitionMap;
        
    public:
        ColoredPetriNetBuilder();
        ColoredPetriNetBuilder(const ColoredPetriNetBuilder& orig);
        virtual ~ColoredPetriNetBuilder();
        
        void addPlace(const std::string& name,
                int tokens,
                double x = 0,
                double y = 0) override ;
        void addPlace(const std::string& name,
                Colored::ColorType* type,
                Colored::Multiset&& tokens,
                double x = 0,
                double y = 0) override;
        void addPlace(const std::string& name,
                      Colored::Multiset&& tokens,
                      Colored::ColorType* type,
                      std::vector<Colored::TimeInvariant>& invariant,
                      double x = 0,
                      double y = 0) override;
        void addTransition(const std::string& name,
                double x = 0,
                double y = 0) override;
        void addTransition(const std::string& name,
                const Colored::GuardExpression_ptr& guard,
                double x = 0,
                double y = 0) override;
        void addTransition(const std::string& name,
                           const Colored::GuardExpression_ptr& guard,
                           bool urgent,
                           double x,
                           double y) override;
        void addInputArc(const std::string& place,
                const std::string& transition,
                bool inhibitor,
                int) override;
         void addInputArc(const std::string &place,
                const std::string &transition,
                bool inhibitor, bool transport,
                const Colored::ArcExpression_ptr& expr,
                std::vector<PetriEngine::Colored::TimeInterval> &interval, 
                int weight, 
                std::string transportID) override;
        void addInputArc(const std::string& place,
                const std::string& transition,
                const Colored::ArcExpression_ptr& expr,
                bool inhibitor,
                int weight) override;

        void addOutputArc(const std::string& transition, 
                          const std::string& place, 
                          const Colored::ArcExpression_ptr& expr, 
                          bool transport, 
                          std::string transportID) override;
        void addOutputArc(const std::string& transition,
                const std::string& place,
                const Colored::ArcExpression_ptr& expr) override;
        void addOutputArc(const std::string& transition,
                          const std::string& place,
                          int weight = 1, bool transport = false, std::string transportID  = "") override;
        void addColorType(const std::string& id,
                Colored::ColorType* type) override;

        void addTransportArc(const std::string& source,
                const std::string& transition,
                const std::string& destination,
                const Colored::ArcExpression_ptr& expr,
                std::vector<Colored::TimeInterval>& interval) override;



        void sort() override;

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

        uint32_t getUnfoldedPlaceCount() const {
            //return _nptplaces;
            return _ptBuilder.numberOfPlaces();
        }

        uint32_t getUnfoldedTransitionCount() const {
            return _ptBuilder.numberOfTransitions();
        }

        uint32_t getUnfoldedArcCount() const {
            return _nptarcs;
        }

        bool isUnfolded() const {
            return _unfolded;
        }

        const PTPlaceMap& getUnfoldedPlaceNames() const {
            return _ptplacenames;
        }

        const PTTransitionMap& getUnfoldedTransitionNames() const {
            return _pttransitionnames;
        }
        
        std::map<std::string, std::string> getInvariants() {return _invariantStrings;}

        PetriNetBuilder& unfold();
        PetriNetBuilder& stripColors();
    private:
        std::map<std::string, std::string> _invariantStrings;
        std::unordered_map<std::string,uint32_t> _placenames;
        std::unordered_map<std::string,uint32_t> _transitionnames;
        PTPlaceMap _ptplacenames;
        std::unordered_map<uint32_t, std::string> _sumPlacesNames;
        PTTransitionMap _pttransitionnames;
        std::vector< std::tuple<double, double> > _placelocations;
        std::vector< std::tuple<double, double> > _transitionlocations;
        uint32_t _nptplaces = 0;
        uint32_t _npttransitions = 0;
        uint32_t _nptarcs = 0;
        
        std::vector<Colored::Transition> _transitions;
        std::vector<Colored::Place> _places;
        std::vector<Colored::Arc> _arcs;
        std::vector<Colored::Arc> inhibitorArcs;
        ColorTypeMap _colors;
        PetriNetBuilder _ptBuilder;
        bool _unfolded = false;
        bool _stripped = false;

        double _time;

        std::string arcToString(Colored::Arc& arc) const ;
        
        void addArc(const std::string& place,
                const std::string& transition,
                const Colored::ArcExpression_ptr& expr,
                bool input, bool inhibitor, int weight);

        void addTimedArc(const std::string& place,
                         const std::string& transition,
                         const Colored::ArcExpression_ptr& expr,
                         std::vector<Colored::TimeInterval>& interval,
                         bool inhibitor, bool transport,
                         bool input, int weight, std::string transportID);
        
        Colored::TimeInterval getTimeIntervalForArc(std::vector< Colored::TimeInterval> timeIntervals,const Colored::Color* color);
        void unfoldPlace(Colored::Place& place);
        Colored::TimeInvariant getTimeInvariantForPlace(std::vector< Colored::TimeInvariant> TimeInvariants, const Colored::Color* color);
        void unfoldTransition(Colored::Transition& transition);
        void unfoldArc(Colored::Arc& arc, Colored::ExpressionContext::BindingMap& binding, std::string& name);
        void unfoldInhibitorArc(std::string &oldname, std::string &newname);
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

