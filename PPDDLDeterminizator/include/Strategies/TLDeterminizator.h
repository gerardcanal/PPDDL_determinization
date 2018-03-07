//
// Created by gcanal on 8/02/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_TLDETERMINIZATION_H
#define ROSPLAN_PLANNING_SYSTEM_TLDETERMINIZATION_H


#include "PPDDLDeterminizator.h"
#include <cmath>
#define ALPHA 1.0
#define BETA 1.0

//Transition-likelihood determinization from Kaelbling and Lozano-Pérez 2013
class TLDeterminizator : public PPDDLDeterminizator {
public:
    using PPDDLDeterminizator::determinize;
    //PPDDLInterface::Domain determinize(const PPDDLInterface::Domain& d);
    PPDDLInterface::ActionPtr determinize(const PPDDLInterface::Action& a) override;
    //PPDDLInterface::EffectPtr determinize(const PPDDLInterface::Effect& e, const PPDDLInterface::Action& a);
    PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ConjunctiveEffect& ce) override ;
    PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ProbabilisticEffect& pe) override;

    TLDeterminizator(double alpha = ALPHA, double beta = BETA);
private:
    double _alpha;
    double _beta;
};


#endif //ROSPLAN_PLANNING_SYSTEM_TLDETERMINIZATION_H
