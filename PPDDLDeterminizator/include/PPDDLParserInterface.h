//
// Created by gcanal on 18/12/17.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H


/**
 * THIS CLASSES ARE AN INTERFACE TO THE MDPSIM PARSER CLASSES, HANDLE THE MEMORY AND EASENS THE INTERFACE. THESE ARE THE
 * ONLY ONES THAT SHOULD BE USED TO INTERACT WITH THE PLANNING DOMAIN!
 */

#include "PPDDLParser/domains.h"


/* The parse function. */
extern int ppddl_parse(); // FIXME namespace this variables!?
/* File to parse. */
extern FILE* yyin;

// Domain class
namespace PPDDLInterface {
// Typedefs
typedef ::Domain p_Domain; // The ::Domain syntax makes it reference to the upper scope namespace i.e. to the MDPSim parser one in this case

// Classes

    class Domain { // TODO copy constructor initialization from a p_Domain?
        public:
            explicit Domain(const std::string& domain_path); // Read domain
            Domain(const PPDDLInterface::Domain& p); // Copy constructor -from a PPDDL domain-
            ~Domain();

        private:
             p_Domain *_dom;

            bool readDomain(const std::string &domain_path, int verbosity=2, int warning_level=1);
            friend std::ostream &operator<<(std::ostream &output, const Domain &D);
    };


    //class Action

}
#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
