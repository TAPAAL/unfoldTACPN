#include "PQL/Visitor.h"

namespace unfoldtacpn
{
    namespace PQL
    {
        // Quantifiers, most uses of the visitor will not use the quantifiers - so we give a default implementation.
            // default behaviour is error
            void Visitor::_accept(const ControlCondition*)
            {   assert(false); std::cerr << "No accept for ControlCondition" << std::endl; exit(0);}
            void Visitor::_accept(const EFCondition*)
            {   assert(false); std::cerr << "No accept for EFCondition" << std::endl; exit(0);}
            void Visitor::_accept(const EGCondition*)
            {   assert(false); std::cerr << "No accept for EGCondition" << std::endl; exit(0);}
            void Visitor::_accept(const AGCondition*)
            {   assert(false); std::cerr << "No accept for AGCondition" << std::endl; exit(0);}
            void Visitor::_accept(const AFCondition*)
            {   assert(false); std::cerr << "No accept for AFCondition" << std::endl; exit(0);}
            void Visitor::_accept(const EXCondition*)
            {   assert(false); std::cerr << "No accept for EXCondition" << std::endl; exit(0);}
            void Visitor::_accept(const AXCondition*)
            {   assert(false); std::cerr << "No accept for AXCondition" << std::endl; exit(0);}
            void Visitor::_accept(const EUCondition*)
            {   assert(false); std::cerr << "No accept for EUCondition" << std::endl; exit(0);}
            void Visitor::_accept(const AUCondition*)
            {   assert(false); std::cerr << "No accept for AUCondition" << std::endl; exit(0);}
            void Visitor::_accept(const PFCondition*)
            {   assert(false); std::cerr << "No accept for PFCondition" << std::endl; exit(0);}
            void Visitor::_accept(const PGCondition*)
            {   assert(false); std::cerr << "No accept for PGCondition" << std::endl; exit(0);}

            // shallow elements, neither of these should exist in a compiled expression
            void Visitor::_accept(const KSafeCondition* element)
            {   assert(false); std::cerr << "No accept for KSafeCondition" << std::endl; exit(0);}
            void Visitor::_accept(const BooleanCondition* element)
            {   assert(false); std::cerr << "No accept for BooleanCondition" << std::endl; exit(0);}
            void Visitor::_accept(const ShallowCondition *element) {
                assert(false);
                std::cerr << "No accept for ShallowCondition" << std::endl;
                exit(0);
            }

            void Visitor::_accept(const BoundExpr*)
            {   assert(false); std::cerr << "No accept for PGCondition" << std::endl; exit(0);}

            // shallow expression, default to error
            void Visitor::_accept(const IdentifierExpr* element)
            {   assert(false); std::cerr << "No accept for IdentifierExpr" << std::endl; exit(0);}

    }
}
