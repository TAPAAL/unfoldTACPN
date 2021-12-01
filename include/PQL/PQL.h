/* PeTe - Petri Engine exTremE
 * Copyright (C) 2011  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                     Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                     Lars Kærlund Østergaard <larsko@gmail.com>,
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
#ifndef PQL_H
#define PQL_H


#include <string>
#include <vector>
#include <memory>

namespace unfoldtacpn {
    namespace PQL {
        class Visitor;
        class NamingContext;

        /** Representation of a PQL error */
        class ExprError : public std::exception {
            std::string _text;
        public:

            const char* what() {
                return _text.c_str();
            }

            ExprError(std::string text = "") {
                _text = text;
            }
        };

        /** Representation of an expression */
        class Expr {
        public:
            virtual ~Expr();
            virtual void analyze(NamingContext& context) = 0;
            virtual void visit(Visitor& visitor) const = 0;
        };

        /** Base condition */
        class Condition {
        public:
            /** Virtual destructor */
            virtual ~Condition();
            virtual void analyze(NamingContext& context) = 0;
            virtual void visit(Visitor& visitor) const = 0;
        };
        typedef std::shared_ptr<Condition> Condition_ptr;
        typedef std::shared_ptr<Expr> Expr_ptr;

        void to_xml(std::ostream&, const Condition&, uint32_t init_tabs = 0, uint32_t tab_size = 2, bool print_newlines = true);
    } // PQL
}

#endif // PQL_H
