#include "PQL/SMCExpressions.h"

#include "PQL/Contexts.h"
#include "PQL/Expressions.h"
#include "errorcodes.h"
#include "PQL/Visitor.h"

namespace unfoldtacpn::PQL {

    void ProbaCondition::analyze(NamingContext &context)
    {
        _cond->analyze(context);
        for(auto& obs : _observables) {
            std::get<1>(obs)->analyze(context);
        }
    }

    void PFCondition::visit(Visitor& ctx) const
    {
        ctx.accept<decltype(this)>(this);
    }

    void PGCondition::visit(Visitor& ctx) const
    {
        ctx.accept<decltype(this)>(this);
    }

}