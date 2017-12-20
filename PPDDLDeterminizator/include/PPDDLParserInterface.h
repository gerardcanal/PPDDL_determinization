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

namespace PPDDLInterface {
// Typedefs
typedef ::Domain p_Domain; // The ::Domain syntax makes it reference to the upper scope namespace i.e. to the MDPSim parser one in this case

// Classes

    class Domain { // TODO copy constructor initialization from a p_Domain?
        public:
            explicit Domain(const std::string& name); // Empty constructor
            explicit Domain(const p_Domain* p); // Copy constructor -from a PPDDL domain-
            ~Domain();

        private:
             p_Domain *_dom;

            friend std::ostream &operator<<(std::ostream &output, const Domain &D);
    };


    //class Action

}
#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
