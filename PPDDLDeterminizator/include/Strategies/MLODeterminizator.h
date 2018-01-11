// Copyright (C) 2010-2017 Institut de Robotica i Informatica Industrial, CSIC-UPC.
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


#ifndef ROSPLAN_PLANNING_SYSTEM_MLODETERMINIZATOR_H
#define ROSPLAN_PLANNING_SYSTEM_MLODETERMINIZATOR_H

#include "PPDDLDeterminizator.h"
#include <memory>

// Most-Likely-Outcome Determinization
class MLODeterminizator : public PPDDLDeterminizator {
public:
    Domain determinize(const Domain& d);
    PPDDLInterface::Domain determinize(const PPDDLInterface::Domain& d);
    ActionSchema determinize(const ActionSchema& as);
    PPDDLInterface::Action determinize(const PPDDLInterface::Action& as);
    const Effect& determinize(const Effect&);
    const PPDDLInterface::Effect determinize(const PPDDLInterface::Effect& e);
    const Effect& determinize(const ConjunctiveEffect&);
    const PPDDLInterface::Effect determinize(const PPDDLInterface::ConjunctiveEffect& ce);
    void determinize(ProbabilisticEffect&);
    const PPDDLInterface::Effect determinize(const PPDDLInterface::ProbabilisticEffect& pe);
};


#endif //ROSPLAN_PLANNING_SYSTEM_MLODETERMINIZATOR_H
