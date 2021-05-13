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
void unfoldNet(std::ifstream& inputModelFile, std::ifstream& inputQueryFile, std::ostream& outputModelFile, std::fstream& outputQueryFile,unfoldtacpn_options_t options);

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


