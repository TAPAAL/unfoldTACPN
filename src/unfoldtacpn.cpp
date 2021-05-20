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
#ifdef VERIFYPN_MC_Simplification
#include <thread>
#include <iso646.h>
#endif
#include "unfoldtacpn.h"
#include "PetriEngine/PQL/PQLParser.h"
#include "PetriEngine/PQL/Contexts.h"
#include "PetriEngine/Reducer.h"
#include "PetriParse/QueryXMLParser.h"
#include "PetriParse/QueryBinaryParser.h"
#include "PetriParse/PNMLParser.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/errorcodes.h"
#include "PetriEngine/PQL/Expressions.h"
#include "PetriEngine/Colored/ColoredPetriNetBuilder.h"

using namespace std;
using namespace PetriEngine;
using namespace PetriEngine::PQL;

ReturnValue contextAnalysis(ColoredPetriNetBuilder& cpnBuilder, PetriNetBuilder& builder, const PetriNet* net, std::vector<std::shared_ptr<Condition> >& queries)
{
    //Context analysis
    ColoredAnalysisContext context(builder.getPlaceNames(), builder.getTransitionNames(), net, cpnBuilder.getUnfoldedPlaceNames(), cpnBuilder.getUnfoldedTransitionNames(), cpnBuilder.isColored());
    for(auto& q : queries)
    {
        q->analyze(context);

        //Print errors if any
        if (context.errors().size() > 0) {
            for (size_t i = 0; i < context.errors().size(); i++) {
                fprintf(stderr, "Query Context Analysis Error: %s\n", context.errors()[i].toString().c_str());
            }
            return ErrorCode;
        }
    }
    return ContinueCode;
}

auto readQueries(unfoldtacpn_options_t& options, std::vector<std::string>& qstrings, ifstream& qfile)
{

    std::vector<Condition_ptr > conditions;
        
        if(options.querynumbers.size() == 0)
        {
            string querystring; // excluding EF and AG

            //Read everything
            stringstream buffer;
            buffer << qfile.rdbuf();
            string querystr = buffer.str(); // including EF and AG
            //Parse XML the queries and querystr let be the index of xmlquery 
        
            qstrings.push_back(querystring);
            //Validate query type
            if (querystr.substr(0, 2) != "EF" && querystr.substr(0, 2) != "AG") {
                    fprintf(stderr, "Error: Query type \"%s\" not supported, only (EF and AG is supported)\n", querystr.substr(0, 2).c_str());
                    return conditions;
            }
            //Check if is invariant
            bool isInvariant = querystr.substr(0, 2) == "AG";

            //Wrap in not if isInvariant
            querystring = querystr.substr(2);
            std::vector<std::string> tmp;
            conditions.emplace_back(ParseQuery(querystring, tmp));
            if(isInvariant) conditions.back() = std::make_shared<AGCondition>(conditions.back());
            else            conditions.back() = std::make_shared<EFCondition>(conditions.back());
        }
        else
        {
            std::vector<QueryItem> queries;
            
            QueryXMLParser parser;
            if (!parser.parse(qfile, options.querynumbers)) {
                fprintf(stderr, "Error: Failed parsing XML query file\n");
                fprintf(stdout, "DO_NOT_COMPETE\n");
                conditions.clear();
                return conditions;
            }
            queries = std::move(parser.queries);
            

            size_t i = 0;
            for(auto& q : queries)
            {
                if(!options.querynumbers.empty()
                        && options.querynumbers.count(i) == 0)
                {
                    ++i;
                    continue;
                }
                ++i;

                if (q.parsingResult == QueryItem::UNSUPPORTED_QUERY) {
                    fprintf(stdout, "The selected query in the XML query file is not supported\n");
                    fprintf(stdout, "FORMULA %s CANNOT_COMPUTE\n", q.id.c_str());
                    continue;
                }
                // fprintf(stdout, "Index of the selected query: %d\n\n", xmlquery);

                conditions.push_back(q.query);
                if (conditions.back() == nullptr) {
                    fprintf(stderr, "Error: Failed to parse query \"%s\"\n", q.id.c_str()); //querystr.substr(2).c_str());
                    fprintf(stdout, "FORMULA %s CANNOT_COMPUTE\n", q.id.c_str());
                    conditions.pop_back();
                }
                
                qstrings.push_back(q.id);
            }
        }
        return conditions;
    
 }

ReturnValue parseModel(AbstractPetriNetBuilder& builder, ifstream& mfile)
{
    //Parse and build the petri net
    PNMLParser parser;
    parser.parse(mfile, &builder);
    //options.isCPN = builder.isColored();

    // Close the file
    mfile.close();
    return ContinueCode;
}


void printUnfoldingStats(ColoredPetriNetBuilder& builder, unfoldtacpn_options_t& options) {
    if (options.printstatistics) {
        if (!builder.isColored() && !builder.isUnfolded())
            return;
        std::cout << "\nSize of colored net: " <<
                builder.getPlaceCount() << " places, " <<
                builder.getTransitionCount() << " transitions, and " <<
                builder.getArcCount() << " arcs" << std::endl;
        std::cout << "Size of unfolded net: " <<
                builder.getUnfoldedPlaceCount() << " places, " <<
                builder.getUnfoldedTransitionCount() << " transitions, and " <<
                builder.getUnfoldedArcCount() << " arcs" << std::endl;
        std::cout << "Unfolded in " << builder.getUnfoldTime() << " seconds" << std::endl;
    }
}
 
void writeQueries(vector<std::shared_ptr<Condition>>& queries, vector<std::string>& querynames, std::vector<uint32_t>& order, fstream& out, const std::unordered_map<std::string, uint32_t>& place_names) 
{
    //out << "<?xml version=\"1.0\"?>\n<property-set xmlns=\"http://mcc.lip6.fr/\">\n";
    
    
    for(uint32_t j = 0; j < queries.size(); j++) {
        auto i = order[j];
        queries[i]->toString(out);
        /*if(queries[i]->isTriviallyTrue())
            queries[i] = std::make_shared<EFCondition>(std::make_shared<BooleanCondition>(true));
        else if(queries[i]->isTriviallyFalse())
            queries[i] = std::make_shared<EFCondition>(std::make_shared<BooleanCondition>(false));
        
        out << "  <property>\n    <id>" << querynames[i] << "</id>\n    <description>Simplified</description>\n    <formula>\n";
        queries[i]->toXML(out, 3);
        out << "    </formula>\n  </property>\n";
        
    
    }
    out << "</property-set>\n";*/ 
    }   
    out.close();
}


void unfoldNet(std::ifstream& inputModelFile, std::ifstream& inputQueryFile, std::ostream& outputModelFile, std::fstream& outputQueryFile,unfoldtacpn_options_t options) {
    ColoredPetriNetBuilder cpnBuilder;
    if(parseModel(cpnBuilder, inputModelFile) != ContinueCode) 
    {
        std::cerr << "Error parsing the model" << std::endl;
        //return ErrorCode;
    }
    if (options.printstatistics) {
        std::cout << "Finished parsing model" << std::endl;
    }
    

    //----------------------- Parse Query -----------------------//
    std::vector<std::string> querynames;
    auto queries = readQueries(options, querynames, inputQueryFile);

    auto builder = cpnBuilder.unfold();
    printUnfoldingStats(cpnBuilder, options);
    builder.sort();
    
    //----------------------- Query Simplification -----------------------//
    PetriNetBuilder b2(builder);
    std::unique_ptr<PetriNet> qnet(b2.makePetriNet(false));

    if(queries.size() == 0 || contextAnalysis(cpnBuilder, b2, qnet.get(), queries) != ContinueCode)
    {
        std::cerr << "Could not analyze the queries" << std::endl;
        //return ErrorCode;
    }
    
    std::vector<uint32_t> reorder(queries.size());
    for(uint32_t i = 0; i < queries.size(); ++i) reorder[i] = i;
    std::sort(reorder.begin(), reorder.end(), [&queries](auto a, auto b){

        if(queries[a]->isReachability() != queries[b]->isReachability())
            return queries[a]->isReachability() > queries[b]->isReachability();
        if(queries[a]->isLoopSensitive() != queries[b]->isLoopSensitive())
            return queries[a]->isLoopSensitive() < queries[b]->isLoopSensitive();
        if(queries[a]->containsNext() != queries[b]->containsNext())
            return queries[a]->containsNext() < queries[b]->containsNext();
        return queries[a]->formulaSize() < queries[b]->formulaSize();
    });

    //Write queries to output file
    writeQueries(queries, querynames, reorder, outputQueryFile, builder.getPlaceNames());

    //Write net to output file
    auto net = std::unique_ptr<PetriNet>(builder.makePetriNet());
    net->toXML(outputModelFile, cpnBuilder.isTimed(), cpnBuilder.getInvariants(), options.outputVerifydtapnFormat);
    
    
       
    //return SuccessCode;
}

