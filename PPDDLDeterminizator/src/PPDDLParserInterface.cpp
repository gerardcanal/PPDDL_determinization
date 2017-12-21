//
// Created by gcanal on 18/12/17.
//

#include "PPDDLParserInterface.h"
#include <cstring>

/* Name of current file. */
std::string current_file;
/* Level of warnings. */
int warning_level;

PPDDLInterface::Domain::Domain(const std::string &path) {
    if (readDomain(path)) {
        std::cout << "Domain file " << path << " parsed correctly." << std::endl;
        int i = 0;
        for (p_Domain::DomainMap::const_iterator di = p_Domain::begin(); di != p_Domain::end(); di++, ++i) {
            if (i > 0) { // Ensures only one iteration
                std::cerr << "More than one domain were found. Only the first one (" << _dom->name() << ") will be used." << std::endl;
                break;
            }
            _dom = const_cast<p_Domain*>(di->second);
            // Note: const_cast is used here as the domain is read here (and only here) and it avoids making a copy by
            //       using the other constructor, as well as avoids removing the const from the original file which I
            //       preferred to modify as less as possible.
        }
    }
    else std::cerr << "There were errors while parsing input file!" << std::endl;
}

PPDDLInterface::Domain::~Domain() {
    delete _dom;
}


PPDDLInterface::Domain::Domain(const PPDDLInterface::Domain& d) {
    const p_Domain* p = d._dom;
    _dom = new p_Domain(p->name()+ "_det");

    /* Domain types. */
    TypeTable types_ = p->types(); // A copy is made here
    _dom->types() = types_;

    /* Domain predicates. */
    PredicateTable predicates_ = p->predicates();
    _dom->predicates() = predicates_;

    /* Domain functions. */
    FunctionTable functions_ = p->functions();
    _dom->functions() = functions_;

    /* Total-time and goal-achieved are created in the Domains creator
    // The `total-time' function. //
    Function total_time_ = p->total_time();
    _dom->total_time() = total_time_;

    // The `goal-achieved' function. //
    Function goal_achieved_ = p->goal_achieved();
    _dom->goal_achieved() = goal_achieved_;*/

    /* Domain terms. */
    TermTable terms_ = p->terms();
    _dom->terms() = terms_;

    /* Domain actions. */
    //ActionSchemaMap actions_; // Copy all the actions!!
    for (ActionSchemaMap::const_iterator ai = p->actions().begin(); ai != p->actions().end(); ai++) {
        ActionSchema* action_ = new ActionSchema(ai->first); // Allocate new actionSchema. TODO It MUST be deleted in the destructor


        action_->set_parameters(ai->second->parameters()); // Set parameters makes the copy

        const StateFormula* precondition = &ai->second->precondition();
        action_->set_precondition(ai->second->precondition().clone()); // Where to add it?*

        action_->set_effect(ai->second->effect().clone());

        _dom->add_action(*action_); // FIXME ensure deletion of above news inside ActionSchema
    }
}

bool PPDDLInterface::Domain::readDomain(const std::string &domain_path, int new_verbosity, int new_warning_level) {
    warning_level = new_warning_level;

    /* Parses the given file, and returns true on success. */
    yyin = fopen(domain_path.c_str(), "r");
    if (yyin == nullptr) {
        std::cerr << "mdpclient:" << domain_path << ": " << strerror(errno)
                  << std::endl;
        return false;
    } else {
        current_file = domain_path;
        bool success = (ppddl_parse() == 0);
        fclose(yyin);
        return success;
    }
}


namespace PPDDLInterface {
    std::ostream &operator<<(std::ostream &output, const PPDDLInterface::Domain &D) {
        output << *D._dom;
        return output;
    }
};
