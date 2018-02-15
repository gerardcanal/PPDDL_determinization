//
// Created by gcanal on 8/02/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_AODETERMINIZATION_H
#define ROSPLAN_PLANNING_SYSTEM_AODETERMINIZATION_H


#include "PPDDLDeterminizator.h"

class AODeterminizator : public PPDDLDeterminizator {
public:
    using PPDDLDeterminizator::determinize;
    //PPDDLInterface::Domain determinize(const PPDDLInterface::Domain& d);
    PPDDLInterface::ActionPtr determinize(const PPDDLInterface::Action& a);
    //PPDDLInterface::EffectPtr determinize(const PPDDLInterface::Effect& e, const PPDDLInterface::Action& a);
    PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ConjunctiveEffect& ce);
    PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ProbabilisticEffect& pe);

    AODeterminizator() : PPDDLDeterminizator("ao") {};
};


#endif //ROSPLAN_PLANNING_SYSTEM_AODETERMINIZATION_H
