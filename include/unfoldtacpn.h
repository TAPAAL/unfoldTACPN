#include <iostream>
#ifndef LIBUNFOLDTACPN_H
#define LIBUNFOLDTACPN_H

#include <ctype.h>
#include <stddef.h>
#include <limits>
#include <set>
#include <string.h>
#include <cctype>
#include <iostream>


struct unfoldtacpn_options_t;
void unfoldNet(std::ifstream& inputModelFile, std::ifstream& inputQueryFile, std::ostream& outputModelFile, std::fstream& outputQueryFile,unfoldtacpn_options_t options){
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

struct unfoldtacpn_options_t {
//    bool outputtrace = false;
    char* modelfile = NULL;
    char* queryfile = NULL;
    bool printstatistics = true;
    std::set<size_t> querynumbers;
    //int queryReductionTimeout = 30;
    uint32_t cores = 1;

    //CTL Specific options
    
    std::string query_out_file;
    std::string model_out_file;

    
    //CPN Specific options
    //bool cpnOverApprox = false;
    bool isCPN = false;

    //TACPN options
    bool outputVerifydtapnFormat = false;

    void print() {
        if (!printstatistics) {
            return;
        }
        
        std::string optionsOut;        
        
        /*if (queryReductionTimeout > 0) {
            optionsOut += ",Query_Simplication=ENABLED,QSTimeout=" + std::to_string(queryReductionTimeout);
        } else {
            optionsOut += ",Query_Simplication=DISABLED";
        }*/
        
        optionsOut += "\n";
        
        std::cout << optionsOut;
    }
};

#endif


