/* Copyright (C) 2011  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                     Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                     Lars Kærlund Østergaard <larsko@gmail.com>,
 *                     Peter Gjøl Jensen <root@petergjoel.dk>,
 *                     Rasmus Tollund <rtollu18@student.aau.dk>
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
#ifndef VERIFYPN_XMLPRINTER_H
#define VERIFYPN_XMLPRINTER_H

#include <iostream>
#include "Visitor.h"

namespace unfoldtacpn {
    namespace PQL {
        class XMLPrinter : public Visitor {
        public:
            XMLPrinter(std::ostream& os, uint32_t init_tabs = 0, uint32_t tab_size = 2, bool print_newlines = true) :
                os(os), tabs(init_tabs), tab_size(tab_size),
                        print_newlines(print_newlines) {}

        protected:
            std::ostream& os;
            uint32_t tabs;
            const uint32_t tab_size;
            const bool print_newlines;
            static constexpr char single_tab = ' ';

            std::ostream& generateTabs();
            std::ostream& newline();

            void openXmlTag(const std::string& tag);
            void closeXmlTag(const std::string &tag);
            void outputLine(const std::string &line);

            void _accept(const NotCondition *element) override;

            void _accept(const AndCondition *element) override;

            void _accept(const OrCondition *element) override;

            void _accept(const LessThanCondition *element) override;

            void _accept(const LessThanOrEqualCondition *element) override;

            void _accept(const EqualCondition *element) override;

            void _accept(const NotEqualCondition *element) override;

            void _accept(const DeadlockCondition *element) override;

            void _accept(const ShallowCondition *element) override;

            void _accept(const EFCondition *condition) override;

            void _accept(const EGCondition *condition) override;

            void _accept(const AGCondition *condition) override;

            void _accept(const AFCondition *condition) override;

            void _accept(const EXCondition *condition) override;

            void _accept(const AXCondition *condition) override;

            void _accept(const EUCondition *condition) override;

            void _accept(const AUCondition *condition) override;

            void _accept(const BooleanCondition *element) override;

            void _accept(const UnfoldedIdentifierExpr *element) override;

            void _accept(const LiteralExpr *element) override;

            void _accept(const PlusExpr *element) override;

            void _accept(const MultiplyExpr *element) override;

            void _accept(const MinusExpr *element) override;

            void _accept(const SubtractExpr *element) override;

            void _accept(const IdentifierExpr *element) override;

        };
    }
}

#endif //VERIFYPN_XMLPRINTER_H
