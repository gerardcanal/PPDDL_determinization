//
// Created by gcanal on 8/02/18.
//

#include "AODeterminization.h"

PPDDLInterface::Effect
AODeterminization::determinize(const PPDDLInterface::ConjunctiveEffect &ce, const PPDDLInterface::Action &a) {
    for (size_t i = 0; i < ce.size(); ++i) {
        PPDDLInterface::Effect ef = determinize(*ce.getConjunct(i), a);
        PPDDLInterface::EffectList* det_cjt_list = dynamic_cast<PPDDLInterface::EffectList*>(&ef);
        if (det_cjt_list != nullptr) {
            // Iterate list and create new list with N
            PPDDLInterface::EffectList ret(det_cjt_list->size());

            for (size_t oi = 0; oi != det_cjt_list->size(); ++oi) {
                PPDDLInterface::ConjunctiveEffect cpy = ce; // Copy effect
                cpy.changeConjunct(*det_cjt_list->getEffect(oi), i); // Set the determinized Effect
                ret.addEffect(cpy, det_cjt_list->getWeight(oi));// Add the effect in the list
            }
            return ret; // We alredy found one probabilistic effect, won't/shouldn't be more..
        }
    }
    return ce; // returning the same on as the action will be removed.
}

PPDDLInterface::Effect
AODeterminization::determinize(const PPDDLInterface::ProbabilisticEffect &pe, const PPDDLInterface::Action &a) {
    PPDDLInterface::EffectList determinized_effects(pe);
    return determinized_effects;/*


    size_t n = pe.size();
    // Find the most likely outcome
    PPDDLInterface::Domain dom(""); // FIXME



    for (size_t o = 0; o < n; ++o) {
        PPDDLInterface::Effect o_eff = pe.getEffect(o);

        PPDDLInterface::Action a_o = a;
        a_o.setName(a_o.getName()+"_o"+std::to_string(o)); // TODO
        Effect eff = a.getEffect();

        a_o.change_effect(pe, o_eff);
        dom->addAction(a_o);

    }
    return pe;*/
}
