/* TAPAAL untimed verification engine verifypn
 * Copyright (C) 2011-2018  Jonas Finnemann Jensen <jopsen@gmail.com>,
 *                          Thomas Søndersø Nielsen <primogens@gmail.com>,
 *                          Lars Kærlund Østergaard <larsko@gmail.com>,
 *                          Jiri Srba <srba.jiri@gmail.com>,
 *                          Peter Gjøl Jensen <root@petergjoel.dk>
 *
 * CTL Extension
 *                          Peter Fogh <peter.f1992@gmail.com>
 *                          Isabella Kaufmann <bellakaufmann93@gmail.com>
 *                          Tobias Skovgaard Jepsen <tobiasj1991@gmail.com>
 *                          Lasse Steen Jensen <lassjen88@gmail.com>
 *                          Søren Moss Nielsen <soren_moss@mac.com>
 *                          Samuel Pastva <daemontus@gmail.com>
 *                          Jiri Srba <srba.jiri@gmail.com>
 *
 * Stubborn sets, query simplification, siphon-trap property
 *                          Frederik Meyer Boenneland <sadpantz@gmail.com>
 *                          Jakob Dyhr <jakobdyhr@gmail.com>
 *                          Peter Gjøl Jensen <root@petergjoel.dk>
 *                          Mads Johannsen <mads_johannsen@yahoo.com>
 *                          Jiri Srba <srba.jiri@gmail.com>
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
#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <memory>
#include <utility>
#include <functional>

#include "unfoldtacpn.h"
#include "PQL/PQLParser.h"
#include "PQL/Contexts.h"
#include "PetriParse/QueryXMLParser.h"
#include "PetriParse/PNMLParser.h"
#include "PQL/PQL.h"
#include "errorcodes.h"
#include "PQL/Expressions.h"
#include "Colored/ColoredPetriNetBuilder.h"

using namespace std;
using namespace unfoldtacpn;
using namespace unfoldtacpn::PQL;

namespace unfoldtacpn {

    void contextAnalysis(ColoredPetriNetBuilder& cpnBuilder, std::vector<std::shared_ptr<Condition> >& queries) {
        //Context analysis
        NamingContext context(cpnBuilder.getUnfoldedPlaceNames(),
            cpnBuilder.getUnfoldedTransitionNames());
        for (auto& q : queries) {
            q->analyze(context);
        }
    }

    std::vector<Condition_ptr> readStringQueries(ColoredPetriNetBuilder& builder, ifstream& qfile) {
        stringstream buffer;
        buffer << qfile.rdbuf();
        string querystring = buffer.str(); // including EF and AG
        std::vector<Condition_ptr> r;
        r.emplace_back(parseQuery(querystring));
        contextAnalysis(builder, r);
        return r;
    }

    std::vector<Condition_ptr> readXMLQueries(ColoredPetriNetBuilder& builder, ifstream& qfile, const std::set<size_t>& to_parse) {
        std::vector<Condition_ptr> conditions;

        QueryXMLParser parser;
        if (!parser.parse(qfile, to_parse)) {
            fprintf(stderr, "Error: Failed parsing XML query file\n");
            fprintf(stdout, "DO_NOT_COMPETE\n");
            conditions.clear();
            return conditions;
        }
        auto queries = std::move(parser.queries);


        size_t i = 0;
        for (auto& q : queries) {
            ++i;
            if (q.parsingResult == QueryItem::UNSUPPORTED_QUERY) {
                fprintf(stdout, "The selected query in the XML query file is not supported\n");
                fprintf(stdout, "FORMULA %s CANNOT_COMPUTE\n", q.id.c_str());
                continue;
            }

            conditions.push_back(q.query);
            if (conditions.back() == nullptr) {
                fprintf(stderr, "Error: Failed to parse query \"%s\"\n", q.id.c_str()); //querystr.substr(2).c_str());
                fprintf(stdout, "FORMULA %s CANNOT_COMPUTE\n", q.id.c_str());
                conditions.pop_back();
            }

        }
        contextAnalysis(builder, conditions);
        return conditions;
    }
}
