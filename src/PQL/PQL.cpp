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
#include "PQL/PQL.h"
#include "PQL/Contexts.h"
#include "PQL/Expressions.h"
#include "PQL/XMLPrinter.h"

namespace unfoldtacpn {
    namespace PQL {

        Expr::~Expr() = default;

        Condition::~Condition() = default;

        void to_xml(std::ostream& stream, const Condition& c, uint32_t init_tabs, uint32_t tab_size, bool print_newlines) {
            XMLPrinter printer(stream, init_tabs, tab_size, print_newlines);
            c.visit(printer);
        }

        void to_xml(std::ostream& out, const std::vector<std::pair<Condition_ptr,std::string>>& queries, uint32_t tab_size, bool print_newlines) {
            out << "<?xml version=\"1.0\"?>\n<property-set xmlns=\"http://mcc.lip6.fr/\">\n";

            for (uint32_t j = 0; j < queries.size(); j++) {
                if(queries[j].first)
                {
                    out << "  <property>\n    <id>" << queries[j].second
                        << "</id>\n    <description>Simplified</description>\n    <formula>\n";
                    to_xml(out, *queries[j].first, 4, tab_size, print_newlines);
                    out << "  </property>\n";
                }
            }

            out << "</property-set>\n";

        }

    } // PQL
} // PetriEngine
