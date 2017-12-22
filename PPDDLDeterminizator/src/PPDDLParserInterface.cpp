//
// Created by gcanal on 18/12/17.
//

#include "PPDDLParserInterface.h"
#include <cstring>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// DOMAIN //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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


PPDDLInterface::Action PPDDLInterface::Domain::getAction(const std::string &name) {
    const p_actionSchema* as = _dom->find_action(name);
    if (as == nullptr) std::runtime_error("ERROR: Unknown action name " + name + ".");
    return PPDDLInterface::Action(as);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Actions /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PPDDLInterface::Action::Action(const ActionSchema *as) : _as(as) {
    const p_Effect* e = &as->effect();

    const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect*>(e);
    if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.
        _action_effect = new PPDDLInterface::ConjunctiveEffect(ce);
    }
    else {
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(e);
        if (pe != nullptr) {
            _action_effect = new PPDDLInterface::ProbabilisticEffect(pe);
        }
        else {
            // it is another effect
            _action_effect = new PPDDLInterface::Effect(e);
        }
    }
}


PPDDLInterface::Action::~Action() {
    delete _action_effect; //FIXME think if should be done here! Can this be called other times? Should be shared_ptr?
}



PPDDLInterface::Effect PPDDLInterface::Action::getEffect() {
    return *_action_effect; // Make use of polymorfism
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Effects ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PPDDLInterface::ProbabilisticEffect::ProbabilisticEffect(const PPDDLInterface::p_ProbabilisticEffect *e) : Effect(e) {
    _pe = e;
}

PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::p_ConjunctiveEffect *e): Effect(e) {
    _ce = e;
}

PPDDLInterface::Effect::Effect(const PPDDLInterface::p_Effect *e) {
    _eff = e;
}
