//
// Created by gcanal on 8/02/18.
//

#include "Strategies/AODeterminizator.h"

PPDDLInterface::EffectPtr
AODeterminizator::determinize(const PPDDLInterface::ConjunctiveEffect &ce) {
    for (size_t i = 0; i < ce.size(); ++i) {
        PPDDLInterface::EffectPtr ef = determinize(*ce.getConjunct(i));
        PPDDLInterface::EffectList* det_cjt_list = dynamic_cast<PPDDLInterface::EffectList*>(ef.get());
        if (det_cjt_list != nullptr) {
            // Iterate list and create new list with N
            PPDDLInterface::EffectList ret(det_cjt_list->size());

            for (size_t oi = 0; oi != det_cjt_list->size(); ++oi) {
                PPDDLInterface::ConjunctiveEffect cpy = ce; // Copy effect
                cpy.changeConjunct(*det_cjt_list->getEffect(oi), i); // Set the determinized Effect
                ret.addEffect(cpy, det_cjt_list->getWeight(oi));// Add the effect in the list
            }
            return PPDDLInterface::makePtr(ret); // We already found one probabilistic effect, won't/shouldn't be more..
        }
    }
    return PPDDLInterface::makePtr(ce); // returning the same on as the action will be removed.
}

PPDDLInterface::EffectPtr
AODeterminizator::determinize(const PPDDLInterface::ProbabilisticEffect &pe) {
    PPDDLInterface::EffectList determinized_effects(pe);
    return PPDDLInterface::makePtr(determinized_effects);
}


PPDDLInterface::ActionPtr AODeterminizator::determinize(const PPDDLInterface::Action &as) {
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