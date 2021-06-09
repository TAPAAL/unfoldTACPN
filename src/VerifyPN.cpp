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
#include "unfoldtacpn.h"
#include "PetriEngine/errorcodes.h"
#include <sstream>
#include <vector>
#include <fstream>

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
ReturnValue parseOptions(int argc, char* argv[], unfoldtacpn_options_t& options)
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
        else if (strcmp(argv[i], "--write-queries") == 0)
        {
            options.query_out_file = std::string(argv[++i]);
        }
        else if (strcmp(argv[i], "--write-simplified") == 0)
        {
            options.query_out_xml_file = std::string(argv[++i]);
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
                    "  --write-queries <filename>         Outputs the queries to the given file in .q format after simplification\n"
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
int main(int argc, char* argv[]) {

    unfoldtacpn_options_t options;
    
    ReturnValue v = parseOptions(argc, argv, options);
    if(v != ContinueCode) return v;
    options.print();

    //Load the model
    std::ifstream mfile(options.modelfile, std::ifstream::in);
    if (!mfile) {
        std::cerr << "Error: Input Model file \""<< options.model_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    //Open query file
    std::ifstream qfile(options.queryfile, std::ifstream::in);
    if (!qfile) {
        std::cerr << "Error: Input Query file \""<< options.queryfile << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    std::fstream outputQueryFile;
    outputQueryFile.open(options.query_out_file, std::ios::out);
    if (!outputQueryFile) {
        std::cerr << "Error: Output Query file \""<< options.query_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }

    std::fstream outputQueryXMLFile;
    outputQueryXMLFile.open(options.query_out_xml_file, std::ios::out);
    if (!outputQueryXMLFile) {
        std::cerr << "Error: Output Query file \""<< options.query_out_xml_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    std::fstream outputModelfile(options.model_out_file, std::ifstream::out);
    if (!outputModelfile) {
        std::cerr << "Error: Output Model file \""<< options.model_out_file << "\" couldn't be opened\n" << std::endl;
        std::cout << "CANNOT_COMPUTE\n" << std::endl;
        return ErrorCode;
    }
    
    unfoldNet(mfile, qfile, outputModelfile, outputQueryFile, outputQueryXMLFile,options);

    return SuccessCode;
}

