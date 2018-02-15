#include <iostream>
#include "PPDDLParserInterface.h"
#include "MLODeterminizator.h"
#include "AODeterminizator.h"
#include "TLDeterminizator.h"

void usage() {
    std::cout << "Usage: ./ppddl_determinizer <strategy> <st_parameters> <domain_file> <problem_files> <output_folder>.\n where:" << std::endl;
    std::cout << "\t<strategy>: mlo (most-likely outcome), ao (all-outcome), tl (transition likelihood) or all for all of them" << std::endl;
    std::cout << "\t<st_parameters>: Parameters for the strategy. Now accepting alpha for the tl strategy" << std::endl;
    std::cout << "\t<domain_file>: Path to the domain file in PPDDL" << std::endl;
    std::cout << "\t<problem_files>: List of paths to problem files for the domain. At least one should be prvoided (if separate from the domain file)" << std::endl;
    std::cout << "\t<output_folder>: Path to the folder in which the output determinized files will be written." << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "Error: wrong number of arguments." << std::endl;
        usage();
        exit(-1);
    }

    std::string output_path(argv[argc-1]); // Last one is the output path
    if (output_path.find(".pddl") != std::string::npos || output_path.find(".ppddl") != std::string::npos) {
        std::cerr << "Error: last argument should be the output path." << std::endl;
        usage();
        exit(-1);
    }

    std::string strategy(argv[1]);
    double alpha = ALPHA;
    int domain_idx = 2;
    try {
        alpha = std::stod(argv[2]);
        domain_idx = 3;
    }
    catch(...) {} // no alpha provided

    std::vector<std::string> problems;
    for (int i = domain_idx+1; i < argc-1; ++i) {
        problems.push_back(argv[i]);
    }
    PPDDLInterface::Domain d(argv[domain_idx], problems);

    bool wrong_strategy = true;
    bool det_all = false;
    if (strategy == "all") det_all = true;
    if (strategy == "mlo" || det_all) {
        MLODeterminizator mld;
        mld.determinize(d).printPDDL(output_path);
        wrong_strategy = false;
    }
    if (strategy == "ao" || det_all) {
        AODeterminizator aod;
        aod.determinize(d).printPDDL(output_path);
        wrong_strategy = false;
    }
    if (strategy == "tl" || det_all) {
        TLDeterminizator tld(alpha);
        tld.determinize(d).printPDDL(output_path);
        wrong_strategy = false;
    }

    if (wrong_strategy) {
        std::cerr << "Error: unknown strategy " << strategy << std::endl;
        usage();
        exit(-1);
    }
    return 1;
}


int old_main(int argc, char **argv) {
    if (argc < 2) {
        std::cout << "Error: Wrong arguments. You must provide an argument with the path to the PPDDL file."
                  << std::endl;
        exit(-1);
    }

    //std::vector<std::string> ppaths; ppaths.push_back(argv[2]);
    //PPDDLInterface::Domain d(argv[1], ppaths);
    PPDDLInterface::Domain d(argv[1]);
    PPDDLInterface::Domain d_copy(d);
    MLODeterminizator mld;
    std::cout << "############################\nDeterminization\n###########################" <<std::endl;
    PPDDLInterface::Domain determinized = mld.determinize(d);

    std::cout << "#######################################################\n#######################################################\n#######################################################" <<std::endl;
    std::cout << "WRAPPED DOMAIN: " << d << std::endl;
    std::cout << "#######################################################\n#######################################################\n#######################################################" <<std::endl;
    std::cout << "COPIED DOMAIN: " << d_copy << std::endl;
    std::cout << "#######################################################\n#######################################################\n#######################################################" <<std::endl;
    std::cout << "DETERMINIZED DOMAIN: " << determinized << std::endl;

    d.printPPDDL("/home/gcanal/Desktop/domain_gen_tests");
    determinized.printPDDL("/home/gcanal/Desktop/domain_gen_tests");
    AODeterminizator aod;
    PPDDLInterface::Domain AODdeterminized = aod.determinize(d);
    AODdeterminized.printPDDL("/home/gcanal/Desktop/domain_gen_tests");


    TLDeterminizator tld;
    PPDDLInterface::Domain TLDdeterminized = tld.determinize(d);
    TLDdeterminized.printPDDL("/home/gcanal/Desktop/domain_gen_tests");

    TLDeterminizator tld1(0.);
    PPDDLInterface::Domain TLDdeterminized1 = tld1.determinize(d);
    TLDdeterminized1.printPDDL("/home/gcanal/Desktop/domain_gen_tests");

    TLDeterminizator tld2(5.5);
    PPDDLInterface::Domain TLDdeterminized2 = tld2.determinize(d);
    TLDdeterminized2.printPDDL("/home/gcanal/Desktop/domain_gen_tests");

    return 19;
}