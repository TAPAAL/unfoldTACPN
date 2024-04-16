/*
 *  Copyright Peter G. Jensen, all rights reserved.
 */

/*
 * File:   Visitor.h
 * Author: Peter G. Jensen <root@petergjoel.dk>
 *
 * Created on April 11, 2020, 1:07 PM
 */

#ifndef VISITOR_H
#define VISITOR_H

#include "PQL/Expressions.h"

#include <type_traits>
#include <cassert>
#include <iostream>

namespace unfoldtacpn
{
    namespace PQL
    {
        class Visitor {
        public:
            Visitor() {}

            template<typename T>
            void accept(T element)
            {
                _accept(element);
            }

        protected:

            virtual void _accept(const NotCondition* element) = 0;
            virtual void _accept(const AndCondition* element) = 0;
            virtual void _accept(const OrCondition* element) = 0;
            virtual void _accept(const LessThanCondition* element) = 0;
            virtual void _accept(const LessThanOrEqualCondition* element) = 0;
            virtual void _accept(const EqualCondition* element) = 0;
            virtual void _accept(const NotEqualCondition* element) = 0;

            virtual void _accept(const DeadlockCondition* element) = 0;

            // Quantifiers, most uses of the visitor will not use the quantifiers - so we give a default implementation.
            // default behaviour is error
            virtual void _accept(const ControlCondition*);
            virtual void _accept(const EFCondition*);
            virtual void _accept(const EGCondition*);
            virtual void _accept(const AGCondition*);
            virtual void _accept(const AFCondition*);
            virtual void _accept(const EXCondition*);
            virtual void _accept(const AXCondition*);
            virtual void _accept(const EUCondition*);
            virtual void _accept(const AUCondition*);
            virtual void _accept(const PFCondition*);
            virtual void _accept(const PGCondition*);

            // shallow elements, neither of these should exist in a compiled expression
            virtual void _accept(const KSafeCondition* element);
            virtual void _accept(const BooleanCondition* element);
            virtual void _accept(const ShallowCondition *element);


            // Expression
            virtual void _accept(const UnfoldedIdentifierExpr* element) = 0;
            virtual void _accept(const LiteralExpr* element) = 0;
            virtual void _accept(const PlusExpr* element) = 0;
            virtual void _accept(const MultiplyExpr* element) = 0;
            virtual void _accept(const MinusExpr* element) = 0;
            virtual void _accept(const SubtractExpr* element) = 0;
            
            // Not used unless for SMC, therefore default implementation (default is error)
            virtual void _accept(const TimeBoundExpr* element);
            virtual void _accept(const StepBoundExpr* element);

            // shallow expression, default to error
            virtual void _accept(const IdentifierExpr* element);
        };
    }
}

#endif /* VISITOR_H */

