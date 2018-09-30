// Copyright (C) 2010-2018 Institut de Robotica i Informatica Industrial, CSIC-UPC.
// Gerard Canal <gcanal@iri.upc.edu> - github.com/gerardcanal
// All rights reserved.
//
// This file is part of iri-ros-pkg
// iri-ros-pkg is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "Strategies/MLODeterminizator.h"

//using namespace PPDDLInterface;

PPDDLInterface::ActionPtr MLODeterminizator::determinize(const PPDDLInterface::Action &as) {
    PPDDLInterface::Action ret(as); // We copy all the action
    ret.setEffect(*determinize(*as.getEffect()));
    return makePtr(ret);
}

/*
 * MLO
 */
 PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::Effect &e) {
    // Check effect type
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

PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::ConjunctiveEffect &ce) {
    PPDDLInterface::ConjunctiveEffect ret(ce);
    for (size_t i = 0; i < ce.size(); ++i) {
        ret.changeConjunct(*determinize(*ce.getConjunct(i)), i); // FIXME optimize copies in changeConjuncts
    }
    return PPDDLInterface::makePtr(ret);
}

PPDDLInterface::EffectPtr MLODeterminizator::determinize(const PPDDLInterface::ProbabilisticEffect &pe) {
    size_t n = pe.size();
    double max_pr = pe.getProbability(0);
    size_t max_i = 0;
    // Find the most likely outcome
    for (size_t o = 1; o < n; ++o) {
        if (pe.getProbability(o) > max_pr) {
            max_pr = pe.getProbability(o);
            max_i = o;
        }
    }
    return PPDDLInterface::makePtr(pe.getEffect(max_i));
}
