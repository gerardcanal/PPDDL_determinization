//
// Created by gcanal on 8/02/18.
//

#include "AODeterminization.h"

PPDDLInterface::EffectPtr
AODeterminization::determinize(const PPDDLInterface::ConjunctiveEffect &ce, const PPDDLInterface::Action &a) {
    for (size_t i = 0; i < ce.size(); ++i) {
        PPDDLInterface::EffectPtr ef = determinize(*ce.getConjunct(i), a);
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
AODeterminization::determinize(const PPDDLInterface::ProbabilisticEffect &pe, const PPDDLInterface::Action &a) {
    PPDDLInterface::EffectList determinized_effects(pe);
    return PPDDLInterface::makePtr(determinized_effects);
}
