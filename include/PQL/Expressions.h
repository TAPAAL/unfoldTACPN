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
#ifndef EXPRESSIONS_H
#define EXPRESSIONS_H


#include "PQL.h"

namespace unfoldtacpn {
    namespace PQL {

        class CompareCondition;
        class NotCondition;
        /******************** EXPRESSIONS ********************/

        /** Base class for all binary expressions */
        class NaryExpr : public Expr {
        protected:
            NaryExpr() {};
        public:

            NaryExpr(std::vector<Expr_ptr>&& exprs) : _exprs(std::move(exprs)) {
            }
            virtual void analyze(NamingContext& context) override;
            auto begin() const { return _exprs.begin(); }
            auto end() const { return _exprs.end(); }
        protected:
            std::vector<Expr_ptr> _exprs;
        };

        class PlusExpr;
        class MultiplyExpr;

        class CommutativeExpr : public NaryExpr
        {
        public:
            friend CompareCondition;
            virtual void analyze(NamingContext& context) override;
        protected:
            CommutativeExpr() {};
            void init(std::vector<Expr_ptr>&& exprs);
        };

        /** Binary plus expression */
        class PlusExpr : public CommutativeExpr {
        public:

            PlusExpr(std::vector<Expr_ptr>&& exprs);
            void visit(Visitor& visitor) const override;
        protected:
        };

        /** Binary minus expression */
        class SubtractExpr : public NaryExpr {
        public:

            SubtractExpr(std::vector<Expr_ptr>&& exprs) : NaryExpr(std::move(exprs))
            {
            }
            void visit(Visitor& visitor) const override;
        protected:
        };

        /** Binary multiplication expression **/
        class MultiplyExpr : public CommutativeExpr {
        public:

            MultiplyExpr(std::vector<Expr_ptr>&& exprs);
            void visit(Visitor& visitor) const override;
        };

        /** Unary minus expression*/
        class MinusExpr : public Expr {
        public:

            MinusExpr(const Expr_ptr expr) {
                _expr = expr;
            }
            void analyze(NamingContext& context) override;
            void visit(Visitor& visitor) const override;

            const Expr_ptr& operator[](size_t i) const { return _expr; };
        private:
            Expr_ptr _expr;
        };

        /** Literal integer value expression */
        class LiteralExpr : public Expr {
        public:

            LiteralExpr(int value) : _value(value) {
            }
            void analyze(NamingContext& context) override;
            void visit(Visitor& visitor) const override;
            int value() const {
                return _value;
            };

        private:
            int _value;
        };


        class IdentifierExpr : public Expr {
        public:
            IdentifierExpr(const std::string& name) : _name(name) {}
            void analyze(NamingContext& context) override;
            void visit(Visitor& visitor) const override;
            const Expr_ptr& compiled() const { return _compiled; }
            const std::string& name() const { return _name; }
        private:
            std::string _name;
            Expr_ptr _compiled;
        };

        /** Identifier expression */
        class UnfoldedIdentifierExpr : public Expr {
        public:
            UnfoldedIdentifierExpr(const std::string& name, int offest)
            : _name(name) {
            }

            UnfoldedIdentifierExpr(const std::string& name) : UnfoldedIdentifierExpr(name, -1) {
            }

            void analyze(NamingContext& context) override;

            const std::string& name() const
            {
                return _name;
            }

            void visit(Visitor& visitor) const override;
        private:
            /** Identifier text */
            std::string _name;
        };

        class ShallowCondition : public Condition
        {
        public:
            void analyze(NamingContext& context) override
            {
                if (_compiled) _compiled->analyze(context);
                else _analyze(context);
            }
            const Condition_ptr& compiled() const { return _compiled; }
        protected:
            virtual void _analyze(NamingContext& context) = 0;
            Condition_ptr _compiled = nullptr;
        };

        /* Not condition */
        class NotCondition : public Condition {
        public:

            NotCondition(const Condition_ptr cond) {
                _cond = cond;
            }
            void analyze(NamingContext& context) override;
            void visit(Visitor&) const override;
            const Condition_ptr& operator[](size_t i) const { return _cond; };
        private:
            Condition_ptr _cond;
        };


        /******************** TEMPORAL OPERATORS ********************/

        class QuantifierCondition : public Condition
        {
            public:
                virtual const Condition_ptr& operator[] (size_t i) const = 0;
        };

        class SimpleQuantifierCondition : public QuantifierCondition {
        public:
            SimpleQuantifierCondition(const Condition_ptr cond) {
                _cond = cond;
            }
            void analyze(NamingContext& context) override;
            virtual const Condition_ptr& operator[] (size_t i) const override { return _cond;}
        protected:
            Condition_ptr _cond;
        };

        // technically quantifies over strategies
        class ControlCondition : public SimpleQuantifierCondition
        {
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class EXCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class EGCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class EFCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class AXCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class AGCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class AFCondition : public SimpleQuantifierCondition {
        public:
            using SimpleQuantifierCondition::SimpleQuantifierCondition;
            void visit(Visitor&) const override;
        };

        class UntilCondition : public QuantifierCondition {
        public:
            UntilCondition(const Condition_ptr cond1, const Condition_ptr cond2) {
                _cond1 = cond1;
                _cond2 = cond2;
            }
            void analyze(NamingContext& context) override;
            virtual const Condition_ptr& operator[] (size_t i) const override
            { if(i == 0) return _cond1; return _cond2;}

        protected:
            Condition_ptr _cond1;
            Condition_ptr _cond2;
        };

        class EUCondition : public UntilCondition {
        public:
            using UntilCondition::UntilCondition;
            void visit(Visitor&) const override;
        };

        class AUCondition : public UntilCondition {
        public:
            using UntilCondition::UntilCondition;
            void visit(Visitor&) const override;
        };

        /******************** CONDITIONS ********************/
        class LogicalCondition : public Condition {
        public:
            void analyze(NamingContext& context) override;
            const Condition_ptr& operator[](size_t i) const
            {
                return _conds[i];
            };
            auto begin() { return _conds.begin(); }
            auto end() { return _conds.end(); }
            auto begin() const { return _conds.begin(); }
            auto end() const { return _conds.end(); }
            bool empty() const { return _conds.size() == 0; }
            bool singular() const { return _conds.size() == 1; }
            size_t size() const { return _conds.size(); }
        protected:
            LogicalCondition() {};
        protected:
            std::vector<Condition_ptr> _conds;
        };

        /* Conjunctive and condition */
        class AndCondition : public LogicalCondition {
        public:
            AndCondition(std::vector<Condition_ptr>&& conds);
            AndCondition(const std::vector<Condition_ptr>& conds);
            AndCondition(Condition_ptr left, Condition_ptr right);

            void visit(Visitor&) const override;
        };

        /* Disjunctive or conditon */
        class OrCondition : public LogicalCondition {
        public:
            OrCondition(std::vector<Condition_ptr>&& conds);
            OrCondition(const std::vector<Condition_ptr>& conds);
            OrCondition(Condition_ptr left, Condition_ptr right);
            void visit(Visitor&) const override;
        };

        /* Comparison conditon */
        class CompareCondition : public Condition {
        public:

            CompareCondition(const Expr_ptr expr1, const Expr_ptr expr2)
            : _expr1(expr1), _expr2(expr2) {}

            void analyze(NamingContext& context) override;
            const Expr_ptr& operator[](uint32_t id) const
            {
                if(id == 0) return _expr1;
                else return _expr2;
            }
        protected:
            Expr_ptr _expr1;
            Expr_ptr _expr2;
        };

        class EqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            void visit(Visitor&) const override;
        };

        /* None equality conditon */
        class NotEqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            void visit(Visitor&) const override;
        };

        /* Less-than conditon */
        class LessThanCondition : public CompareCondition {
        public:
            using CompareCondition::CompareCondition;
            void visit(Visitor&) const override;
        };

        /* Less-than-or-equal conditon */
        class LessThanOrEqualCondition : public CompareCondition {
        public:

            using CompareCondition::CompareCondition;
            void visit(Visitor&) const override;
        };

        /* Bool condition */
        class BooleanCondition : public Condition {
        public:

            BooleanCondition(bool value) : _value(value) {
            }
            void analyze(NamingContext& context) override;
            void visit(Visitor&) const override;
            static Condition_ptr TRUE_CONSTANT;
            static Condition_ptr FALSE_CONSTANT;
            static Condition_ptr getShared(bool val);
            bool value() const { return _value; }
        private:
            const bool _value;
        };

        /* Deadlock condition */
        class DeadlockCondition : public Condition {
        public:

            DeadlockCondition() {
            }
            void analyze(NamingContext& context) override;
            void visit(Visitor&) const override;
            static Condition_ptr DEADLOCK;
        };

        class KSafeCondition : public ShallowCondition
        {
        public:
            KSafeCondition(const Expr_ptr& expr1) : _bound(expr1)
            {}

        protected:
            void _analyze(NamingContext& context) override;
            void visit(Visitor&) const override;
        private:
            Expr_ptr _bound = nullptr;
        };

    }
}



#endif // EXPRESSIONS_H
