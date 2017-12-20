//
// Created by gcanal on 18/12/17.
//

#include "PPDDLParserInterface.h"

PPDDLInterface::Domain::Domain(const std::string &name) {
    _dom = new p_Domain(name);
}

PPDDLInterface::Domain::~Domain() {
    delete _dom;
}


PPDDLInterface::Domain::Domain(const PPDDLInterface::p_Domain *p) {
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
        std::cout << ai->second->precondition() << std::endl;
        action_->set_precondition(ai->second->precondition().clone()); // Where to add it?*

        action_->set_effect(ai->second->effect().clone());

        _dom->add_action(*action_); // FIXME ensure deletion of above news inside ActionSchema
    }
}

namespace PPDDLInterface {
    std::ostream &operator<<(std::ostream &output, const PPDDLInterface::Domain &D) {
        output << *D._dom;
        return output;
    }
};
