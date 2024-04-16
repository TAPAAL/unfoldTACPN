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
#include "PQL/Contexts.h"
#include "PQL/Expressions.h"
#include "errorcodes.h"
#include "PQL/Visitor.h"

namespace unfoldtacpn {
    namespace PQL {

        // CONSTANTS
        Condition_ptr BooleanCondition::FALSE_CONSTANT = std::make_shared<BooleanCondition>(false);
        Condition_ptr BooleanCondition::TRUE_CONSTANT = std::make_shared<BooleanCondition>(true);
        Condition_ptr DeadlockCondition::DEADLOCK = std::make_shared<DeadlockCondition>();


        Condition_ptr BooleanCondition::getShared(bool val)
        {
            if(val)
            {
                return TRUE_CONSTANT;
            }
            else
            {
                return FALSE_CONSTANT;
            }
        }


        /******************** Context Analysis ********************/

        void NaryExpr::analyze(NamingContext& context) {
            for(auto& e : _exprs) e->analyze(context);
        }

        void CommutativeExpr::analyze(NamingContext& context) {
            NaryExpr::analyze(context);
        }

        void MinusExpr::analyze(NamingContext& context) {
            _expr->analyze(context);
        }

        void LiteralExpr::analyze(NamingContext&) {
            return;
        }

        Expr_ptr generateUnfoldedIdentifierExpr(NamingContext& context, std::unordered_map<uint32_t,std::string>& names, uint32_t colorIndex) {
            const std::string& place = names[colorIndex];
            return std::make_shared<UnfoldedIdentifierExpr>(place);
        }

        void IdentifierExpr::analyze(NamingContext &context) {

            std::unordered_map<uint32_t,std::string> names;
            if (!context.resolvePlace(_name, names)) {
                ExprError error("Unable to resolve colored identifier \"" + _name + "\"");
                throw error;
            }

            if (names.size() == 1) {
                _compiled = generateUnfoldedIdentifierExpr(context, names, 0);
            } else {
                std::vector<Expr_ptr> identifiers;
                for (auto& unfoldedName : names) {
                    identifiers.push_back(generateUnfoldedIdentifierExpr(context,names,unfoldedName.first));
                }
                _compiled = std::make_shared<PQL::PlusExpr>(std::move(identifiers));
            }
        }

        void UnfoldedIdentifierExpr::analyze(NamingContext& context) {
        }

        void SimpleQuantifierCondition::analyze(NamingContext& context) {
            _cond->analyze(context);
        }

        void UntilCondition::analyze(NamingContext& context) {
            _cond1->analyze(context);
            _cond2->analyze(context);
        }

        void ProbaCondition::analyze(NamingContext &context)
        {
            _bound->analyze(context);
            _cond->analyze(context);
        }

        /*void ProbaCompCondition::analyze(NamingContext &context) 
        {
            _cond1->analyze(context);
            _cond2->analyze(context);
        }*/

        void LogicalCondition::analyze(NamingContext& context) {
            for(auto& c : _conds) c->analyze(context);
        }

        void CompareCondition::analyze(NamingContext& context) {
            _expr1->analyze(context);
            _expr2->analyze(context);
        }

        void NotCondition::analyze(NamingContext& context) {
            _cond->analyze(context);
        }

        void BooleanCondition::analyze(NamingContext&) {
        }

        void DeadlockCondition::analyze(NamingContext& c) {
        }

        void KSafeCondition::_analyze(NamingContext &context) {
            std::vector<Condition_ptr> k_safe;
            for(auto& p : context.allColoredPlaceNames())
                k_safe.emplace_back(std::make_shared<LessThanOrEqualCondition>(std::make_shared<IdentifierExpr>(p.first), _bound));
            _compiled = std::make_shared<AGCondition>(std::make_shared<AndCondition>(std::move(k_safe)));
            _compiled->analyze(context);
        }

        void ControlCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void EGCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void EUCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void EXCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void EFCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void AUCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void AXCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void AFCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void AGCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void PFCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void PGCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void AndCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void OrCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void NotCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void EqualCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void NotEqualCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void LessThanOrEqualCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void LessThanCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void BooleanCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void DeadlockCondition::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void KSafeCondition::visit(Visitor& ctx) const
        {
            if(_compiled)
                _compiled->visit(ctx);
            else
                ctx.accept<decltype(this)>(this);
        }

        void LiteralExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void IdentifierExpr::visit(Visitor& ctx) const
        {
            if(_compiled)
                _compiled->visit(ctx);
            else
                ctx.accept<decltype(this)>(this);
        }

        void UnfoldedIdentifierExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }        

        void MinusExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void SubtractExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void PlusExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void MultiplyExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

        void BoundExpr::visit(Visitor& ctx) const
        {
            ctx.accept<decltype(this)>(this);
        }

/********************** CONSTRUCTORS *********************************/

        AndCondition::AndCondition(std::vector<Condition_ptr>&& conds) {
            _conds = std::move(conds);
        }

        AndCondition::AndCondition(const std::vector<Condition_ptr>& conds) {
            _conds = conds;
        }

        AndCondition::AndCondition(Condition_ptr left, Condition_ptr right) {
            _conds = {std::move(left), std::move(right)};
        }

        OrCondition::OrCondition(std::vector<Condition_ptr>&& conds) {
            _conds = std::move(conds);
        }

        OrCondition::OrCondition(const std::vector<Condition_ptr>& conds) {
            _conds = conds;
        }

        OrCondition::OrCondition(Condition_ptr left, Condition_ptr right) {
            _conds = {std::move(left), std::move(right)};
        }

        void CommutativeExpr::init(std::vector<Expr_ptr>&& exprs)
        {
            _exprs = std::move(exprs);
        }

        PlusExpr::PlusExpr(std::vector<Expr_ptr>&& exprs) : CommutativeExpr()
        {
            init(std::move(exprs));
        }

        MultiplyExpr::MultiplyExpr(std::vector<Expr_ptr>&& exprs) : CommutativeExpr()
        {
            init(std::move(exprs));
        }

    } // PQL
}


