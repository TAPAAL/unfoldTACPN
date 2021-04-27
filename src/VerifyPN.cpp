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

#include "PetriEngine/PQL/PQLParser.h"
#include "PetriEngine/PQL/Contexts.h"
#include "PetriEngine/Reducer.h"
#include "PetriParse/QueryXMLParser.h"
#include "PetriParse/QueryBinaryParser.h"
#include "PetriParse/PNMLParser.h"
#include "PetriEngine/PetriNetBuilder.h"
#include "PetriEngine/PQL/PQL.h"
#include "PetriEngine/options.h"
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

std::vector<std::string> explode(std::string const & s)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, ','); )
    {
        result.push_back(std::move(token));
        if(result.back().empty()) result.pop_back();
    }

    return result;
}

ReturnValue parseOptions(int argc, char* argv[], options_t& options)
{
    for (int i = 1; i < argc; i++) {
        /*if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--query-reduction") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "Missing number after \"%s\"\n\n", argv[i]);
                return ErrorCode;
            }
            if (sscanf(argv[++i], "%d", &options.queryReductionTimeout) != 1 || options.queryReductionTimeout < 0) {
                fprintf(stderr, "Argument Error: Invalid query reduction timeout argument \"%s\"\n", argv[i]);
                return ErrorCode;
            }
        } else */if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-statistics") == 0) {
            options.printstatistics = false;
        } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--xml-queries") == 0) {
            if (i == argc - 1) {
                fprintf(stderr, "Missing number after \"%s\"\n\n", argv[i]);
                return ErrorCode;
            }
            std::vector<std::string> q = explode(argv[++i]);
            for(auto& qn : q)
            {
                int32_t n;
                if(sscanf(qn.c_str(), "%d", &n) != 1 || n <= 0)
                {
                    std::cerr << "Error in query numbers : " << qn << std::endl;
                }
                else
                {
                    options.querynumbers.insert(--n);
                }
            }
        }  
        else if (strcmp(argv[i], "--write-simplified") == 0)
        {
            options.query_out_file = std::string(argv[++i]);
        }
        else if (strcmp(argv[i], "--write-reduced") == 0)
        {
            options.model_out_file = std::string(argv[++i]);
        }
        else if (strcmp(argv[i], "-verifydtapn") == 0 ) {
            std::string outputOption = argv[++i];
            if (outputOption.compare("tt") == 0 ) {
                options.outputVerifydtapnFormat = true;
            }
        }
#ifdef VERIFYPN_MC_Simplification
        else if (strcmp(argv[i], "-z") == 0)
        {
            if (i == argc - 1) {
                fprintf(stderr, "Missing number after \"%s\"\n\n", argv[i]);
                return ErrorCode;
            }
            if (sscanf(argv[++i], "%u", &options.cores) != 1) {
                fprintf(stderr, "Argument Error: Invalid cores count \"%s\"\n", argv[i]);
                return ErrorCode;
            }
        }
#endif
        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: verifypn [options] model-file query-file\n"
                    "A tool for answering CTL and reachability queries of place cardinality\n" 
                    "deadlock and transition fireability for weighted P/T Petri nets\n" 
                    "extended with inhibitor arcs.\n"
                    "\n"
                    "Options:\n"
                    "  --seed-offset <number>             Extra noise to add to the seed of the random number generation\n"
                    "  -x, --xml-query <query index>      Parse XML query file and verify query of a given index\n"
                    "  -q, --query-reduction <timeout>    Query reduction timeout in seconds (default 30)\n"
                    "                                     write -q 0 to disable query reduction\n"
                    "  -n, --no-statistics                Do not display any statistics (default is to display it)\n"
                    "  -h, --help                         Display this help message\n"
                    "  -v, --version                      Display version information\n"
#ifdef VERIFYPN_MC_Simplification
                    "  -z <number of cores>               Number of cores to use (currently only query simplification)\n"
#endif
                    "  -verifydtapn <tt>                  Specifies the format of the output model (tt is currently the only valid value)\n"
                    "                                     - tt output as dtapn format\n"
                    "  --write-simplified <filename>      Outputs the queries to the given file after simplification\n"
                    "  --write-reduced <filename>         Outputs the model to the given file after structural reduction\n"
                    "Return Values:\n"
                    "  A unfolded net \n"
                    "\n"
                    "VerifyPN is an untimed CTL verification engine for TAPAAL.\n"
                    "TAPAAL project page: <http://www.tapaal.net>\n");
            return SuccessCode;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("VerifyPN (untimed verification engine for TAPAAL) %s\n", VERIFYPN_VERSION);
            printf("Copyright (C) 2011-2020\n");
            printf("                        Peter Gjøl Jensen <root@petergjoel.dk>\n");
            printf("                        Thomas Pedersen <thomas.pedersen@stofanet.dk\n");
            printf("                        Jiri Srba <srba.jiri@gmail.com>\n");
            printf("GNU GPLv3 or later <http://gnu.org/licenses/gpl.html>\n");
            return SuccessCode;
        } else if (options.modelfile == NULL) {
            options.modelfile = argv[i];
        } else if (options.queryfile == NULL) {
            options.queryfile = argv[i];
        } else {
			fprintf(stderr, "Argument Error: Unrecognized option \"%s\"\n", options.modelfile);
			return ErrorCode;
        }
    }
    //Print parameters
    if (options.printstatistics) {
        std::cout << std::endl << "Parameters: ";
        for (int i = 1; i < argc; i++) {
            std::cout << argv[i] << " ";
        }
        std::cout << std::endl;
    }

    //----------------------- Validate Arguments -----------------------//

    //Check for model file
    if (!options.modelfile) {
        fprintf(stderr, "Argument Error: No model-file provided\n");
        return ErrorCode;
    }

    //Check for query file
    if (!options.queryfile) {
        fprintf(stderr, "Argument Error: No query-file provided\n");
        return ErrorCode;
    }
    
    return ContinueCode;
}

auto
readQueries(options_t& options, std::vector<std::string>& qstrings, ifstream& qfile)
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


void printUnfoldingStats(ColoredPetriNetBuilder& builder, options_t& options) {
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
    out << "<?xml version=\"1.0\"?>\n<property-set xmlns=\"http://mcc.lip6.fr/\">\n";
    
    
    for(uint32_t j = 0; j < queries.size(); j++) {
        auto i = order[j];
        if(queries[i]->isTriviallyTrue())
            queries[i] = std::make_shared<EFCondition>(std::make_shared<BooleanCondition>(true));
        else if(queries[i]->isTriviallyFalse())
            queries[i] = std::make_shared<EFCondition>(std::make_shared<BooleanCondition>(false));
        
        out << "  <property>\n    <id>" << querynames[i] << "</id>\n    <description>Simplified</description>\n    <formula>\n";
        queries[i]->toXML(out, 3);
        out << "    </formula>\n  </property>\n";
        

    }
    out << "</property-set>\n";    
    out.close();
}

void unfoldNet(ifstream& inputModelFile, ifstream& inputQueryFile, ostream& outputModelFile, fstream& outputQueryFile,options_t options) {
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
    
    /*if(options.printstatistics && options.queryReductionTimeout > 0)
    {
        negstat_t stats;            
        std::cout << "RWSTATS LEGEND:";
        stats.printRules(std::cout);            
        std::cout << std::endl;
    }*/

    /*if(cpnBuilder.isColored())
    {
        negstat_t stats;            
        EvaluationContext context(nullptr, nullptr);
        for (ssize_t qid = queries.size() - 1; qid >= 0; --qid) {
            queries[qid] = queries[qid]->pushNegation(stats, context, false, false, false);
            if(options.printstatistics)
            {
                std::cout << "\nQuery before expansion and reduction: ";
                queries[qid]->toString(std::cout);
                std::cout << std::endl;

                std::cout << "RWSTATS COLORED PRE:";
                stats.print(std::cout);
                std::cout << std::endl;
            }
        }
    }*/

    
    auto builder = cpnBuilder.unfold();
    printUnfoldingStats(cpnBuilder, options);
    builder.sort();
    
    //----------------------- Query Simplification -----------------------//
    //bool alldone = options.queryReductionTimeout > 0;
    PetriNetBuilder b2(builder);
    std::unique_ptr<PetriNet> qnet(b2.makePetriNet(false));
    //MarkVal* qm0 = qnet->makeInitialMarking();

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

int main(int argc, char* argv[]) {

    options_t options;
    
    ReturnValue v = parseOptions(argc, argv, options);
    if(v != ContinueCode) return v;
    options.print();

    //Load the model
    ifstream mfile(options.modelfile, ifstream::in);
    if (!mfile) {
        std::cerr << "Error: Query file \""<< options.model_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    //Open query file
    ifstream qfile(options.queryfile, ifstream::in);
    if (!qfile) {
        std::cerr << "Error: Query file \""<< options.queryfile << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    fstream outputQueryFile;
    outputQueryFile.open(options.query_out_file, std::ios::out);
    if (!outputQueryFile) {
        std::cerr << "Error: Query file \""<< options.query_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    fstream outputModelfile(options.model_out_file, ifstream::out);
    if (!outputModelfile) {
        std::cerr << "Error: Query file \""<< options.model_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    
    unfoldNet(mfile, qfile, outputModelfile, outputQueryFile,options);

    return SuccessCode;
}

