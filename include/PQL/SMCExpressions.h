#ifndef SMCEXPRESSIONS_H
#define SMCEXPRESSIONS_H

#include "Expressions.h"

namespace unfoldtacpn::PQL {

    struct SMCSettings {
        int timeBound;
        int stepBound;
        float falsePositives;
        float falseNegatives;
        float indifferenceRegionUp;
        float indifferenceRegionDown;
        float confidence;
        float estimationIntervalWidth;
        bool compareToFloat;
        float geqThan;
    };

    typedef std::tuple<std::string, Expr_ptr> Observable;

    class ProbaCondition : public SimpleQuantifierCondition {
    public:
        ProbaCondition(SMCSettings settings, Condition_ptr cond)
        : SimpleQuantifierCondition(cond) {
            _settings = settings;
        }
        void analyze(NamingContext& context) override;
        void setObservables(std::vector<Observable> obs) { _observables = obs; }
        const std::vector<Observable>& getObservables() const { return _observables; }
        virtual const SMCSettings settings() const { return _settings; }
    protected:
        SMCSettings _settings;
        std::vector<Observable> _observables;
    };

    class PFCondition : public ProbaCondition {
    public:
        using ProbaCondition::ProbaCondition;
        void visit(Visitor&) const override;
    };

    class PGCondition : public ProbaCondition {
    public:
        using ProbaCondition::ProbaCondition;
        void visit(Visitor&) const override;
    };

}

#endif