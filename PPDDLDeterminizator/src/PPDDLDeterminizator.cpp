//
// Created by gcanal on 8/02/18.
//

#include "PPDDLDeterminizator.h"

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H

#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H

PPDDLInterface::Domain PPDDLDeterminizator::determinize(const PPDDLInterface::Domain &d) {
    PPDDLInterface::Domain d_det(d, "det"); // Copy the domain

    std::vector<PPDDLInterface::Action> actions = d_det.getActions();
    for (PPDDLInterface::Domain::action_iterator it = actions.begin(); it != actions.end(); ++it) {
        PPDDLInterface::Action det_action = determinize(*it);
        d_det.setAction(det_action);
    }
    //std::cout << "----------------------\n" << d_det << std::endl;

    return d_det;
}

PPDDLInterface::Action PPDDLDeterminizator::determinize(const PPDDLInterface::Action &as) {
    PPDDLInterface::Action ret(as); // We copy all the action
    ret.setEffect(determinize(*as.getEffect(), as));
    return ret;
}

PPDDLInterface::Effect
PPDDLDeterminizator::determinize(const PPDDLInterface::Effect &e, const PPDDLInterface::Action &a) {    // Check effect type
    const PPDDLInterface::ProbabilisticEffect* pe = dynamic_cast<const PPDDLInterface::ProbabilisticEffect*>(&e);
    if (pe != nullptr) { // Then it's probabilistic
        return determinize(*pe, a);
    }

    const PPDDLInterface::ConjunctiveEffect* ce = dynamic_cast<const PPDDLInterface::ConjunctiveEffect*>(&e);
    if (ce != nullptr) { // It's a conjunctive effect which may have a probabilistic effect in the conjunction
        return determinize(*ce, a);
    }
    return e;
}
