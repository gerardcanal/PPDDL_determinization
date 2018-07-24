//
// Created by gcanal on 8/02/18.
//

#include "Strategies/TLDeterminizator.h"
#include <exception>

PPDDLInterface::EffectPtr
TLDeterminizator::determinize(const PPDDLInterface::ConjunctiveEffect &ce) {
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
TLDeterminizator::determinize(const PPDDLInterface::ProbabilisticEffect &pe) {
    PPDDLInterface::EffectList determinized_effects(pe);
    return PPDDLInterface::makePtr(determinized_effects);
}


PPDDLInterface::ActionPtr TLDeterminizator::determinize(const PPDDLInterface::Action &as) {
    PPDDLInterface::Action ret(as); // We copy all the action
    PPDDLInterface::EffectPtr ep = determinize(*as.getEffect());
    PPDDLInterface::EffectList* el = dynamic_cast<PPDDLInterface::EffectList*>(ep.get());
    if (el != nullptr) { // Then the effect got split in multiple effects.. thus we have to create an ActionList with each effect in a different action
        PPDDLInterface::ActionList al(el->size());
        for (size_t i = 0 ; i < el->size(); ++i) {
            // create new action
            PPDDLInterface::Action a(as, "_d"+std::to_string(i+1));
            a.setEffect(*el->getEffect(i)); // TODO use weight!

            std::string metric = PPDDLInterface::Domain::getMetric();
            bool maximize;
            if (metric == "") {
                maximize = false;
                metric = "total-cost"; // Default metric
            }
            else {
                maximize = metric[0] == '+';
                metric = metric.substr(1);
            }

            try { // It fails if it is not a simple increase by value
                double cost = _alpha * a.getCost(metric) - _beta * log(el->getWeight(i));
                a.setCost(cost, metric);
            }
            catch (std::exception& e) { // Weight a cist function
                PPDDLInterface::p_Update *u = a.getCostFunction(metric);
                const int multiplier = 1000000;
                ppddl_parser::Value* alpha = new ppddl_parser::Value(ppddl_parser::Rational(_alpha*multiplier, multiplier));
                const ppddl_parser::Expression* timesalpha = &ppddl_parser::Multiplication::make(*alpha, u->expression());
                ppddl_parser::Value *betaprob = new ppddl_parser::Value(ppddl_parser::Rational(-_beta*log(el->getWeight(i))*multiplier, multiplier));
                const ppddl_parser::Expression* newcost = &ppddl_parser::Addition::make(*timesalpha, *betaprob);
                u->setExpression(newcost);
                a.setCostFunction(u, metric);
            }
            al.addAction(a);
       }
       return makePtr(al);
    }
    else ret.setEffect(*ep);
    return makePtr(ret);
}

TLDeterminizator::TLDeterminizator(double alpha, double beta) : PPDDLDeterminizator("tl") {
    _alpha = alpha;
    _beta = beta;
}
