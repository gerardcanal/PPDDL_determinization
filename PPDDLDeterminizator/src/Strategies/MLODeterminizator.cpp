// Copyright (C) 2010-2018 Institut de Robotica i Informatica Industrial, CSIC-UPC.
// Gerard Canal <gcanal@iri.upc.edu> - github.com/gerardcanal
// All rights reserved.
//
// This file is part of iri-ros-pkg
// iri-ros-pkg is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "MLODeterminizator.h"
#include "PPDDLParserInterface.h"
#include "AODeterminization.h"
#include "TLDeterminization.h"

//using namespace PPDDLInterface;

PPDDLInterface::Domain MLODeterminizator::determinize(const PPDDLInterface::Domain &d) {
    PPDDLInterface::Domain d_det(d, "det"); // Copy the domain

    std::vector<PPDDLInterface::Action> actions = d_det.getActions();
    for (PPDDLInterface::Domain::action_iterator it = actions.begin(); it != actions.end(); ++it) {
        PPDDLInterface::Action det_action = *determinize(*it);
        d_det.setAction(det_action);
    }
    //std::cout << "----------------------\n" << d_det << std::endl;

    return d_det;
}

PPDDLInterface::ActionPtr MLODeterminizator::determinize(const PPDDLInterface::Action &as) {
    PPDDLInterface::Action ret(as); // We copy all the action
    ret.setEffect(*determinize(*as.getEffect()));
    return makePtr(ret);
}

/*
 * MLO
 */
 PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::Effect &e) {
    // Check effect type
    const PPDDLInterface::ProbabilisticEffect* pe = dynamic_cast<const PPDDLInterface::ProbabilisticEffect*>(&e);
    if (pe != nullptr) { // Then it's probabilistic
        return determinize(*pe);
    }

    const PPDDLInterface::ConjunctiveEffect* ce = dynamic_cast<const PPDDLInterface::ConjunctiveEffect*>(&e);
    if (ce != nullptr) { // It's a conjunctive effect which may have a probabilistic effect in the conjunction
        return determinize(*ce);
    }
    return PPDDLInterface::makePtr(e);
}

PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::ConjunctiveEffect &ce) {
    PPDDLInterface::ConjunctiveEffect ret(ce);
    for (size_t i = 0; i < ce.size(); ++i) {
        ret.changeConjunct(*determinize(*ce.getConjunct(i)), i); // FIXME optimize copies in changeConjuncts
    }
    return PPDDLInterface::makePtr(ret);
}

PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::ProbabilisticEffect &pe) {
    size_t n = pe.size();
    double max_pr = pe.getProbability(0);
    size_t max_i = 0;
    // Find the most likely outcome
    for (size_t o = 1; o < n; ++o) {
        if (pe.getProbability(o) > max_pr) {
            max_pr = pe.getProbability(o);
            max_i = o;
        }
    }
    return PPDDLInterface::makePtr(pe.getEffect(max_i));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <fstream>
/* The parse function. */
   // extern int ppddl_parse(); // FIXME namespace this variables!?
/* File to parse. */
   // extern FILE* yyin;
/* Name of current file. */
    extern std::string current_file;
/* Level of warnings. */
//    extern int warning_level;
/* Verbosity level. */
    //extern int verbosity;

/* Parses the given file, and returns true on success. */
    static bool read_file(const char* name) {
    ppddl_in = fopen(name, "r");
        if (ppddl_in == 0) {
            std::cerr << "mdpclient:" << name << ": " << strerror(errno)
                      << std::endl;
            return false;
        } else {
            current_file = name;
            bool success = (ppddl_parse() == 0);
            fclose(ppddl_in);
            return success;
        }
    }

    int main(int argc, char **argv) {
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
        AODeterminization aod;
        PPDDLInterface::Domain AODdeterminized = aod.determinize(d);
        AODdeterminized.printPDDL("/home/gcanal/Desktop/domain_gen_tests");


        TLDeterminization tld;
        PPDDLInterface::Domain TLDdeterminized = tld.determinize(d);
        TLDdeterminized.printPDDL("/home/gcanal/Desktop/domain_gen_tests");
        return 19;
    }
