//
// Created by gcanal on 8/02/18.
//

#include "PPDDLDeterminizator.h"

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H

#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_CPP_H

PPDDLInterface::Domain PPDDLDeterminizator::determinize(const PPDDLInterface::Domain &d) {
    PPDDLInterface::Domain d_det(d, _method_name_suffix + "det"); // Copy the domain

    std::vector<PPDDLInterface::Action> actions = d_det.getActions();
    for (PPDDLInterface::Domain::action_iterator it = actions.begin(); it != actions.end(); ++it) {
        PPDDLInterface::ActionPtr det_action = determinize(*it);
        PPDDLInterface::ActionList* al = dynamic_cast<PPDDLInterface::ActionList*>(det_action.get());
        if (al != nullptr) {
            d_det.deleteAction(*it); // Delete action as it has been converted to multiple new actions
            for (size_t ai = 0 ; ai != al->size(); ++ai) {
                d_det.setAction(*al->getAction(ai));
            }
        }
        else d_det.setAction(*det_action);
    }

    return d_det;
}

PPDDLInterface::ActionPtr PPDDLDeterminizator::determinize(const PPDDLInterface::Action &as) {
    PPDDLInterface::Action ret(as); // We copy all the action
    PPDDLInterface::EffectPtr ep = determinize(*as.getEffect());
    PPDDLInterface::EffectList* el = dynamic_cast<PPDDLInterface::EffectList*>(ep.get());
    if (el != nullptr) { // Then the effect got split in multiple effects.. thus we have to create an ActionList with each effect in a different action
        PPDDLInterface::ActionList al(el->size());
        for (size_t i = 0 ; i < el->size(); ++i) {
            // create new action
            PPDDLInterface::Action a(as, "_d"+std::to_string(i+1));
            a.setEffect(*el->getEffect(i));
            al.addAction(a);
        }
        return makePtr(al);
    }
    else ret.setEffect(*ep);
    return makePtr(ret);
}

PPDDLInterface::EffectPtr
PPDDLDeterminizator::determinize(const PPDDLInterface::Effect &e) {    // Check effect type
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

PPDDLDeterminizator::PPDDLDeterminizator(string method_suffix) : _method_name_suffix(method_suffix) {}



