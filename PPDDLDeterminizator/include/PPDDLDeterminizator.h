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
//
// IMPORTANT NOTE: This code has been generated through a script from the
// iri_ros_scripts. Please do NOT delete any comments to guarantee the correctness
// of the scripts. ROS topics can be easly add by using those scripts. Please
// refer to the IRI wiki page for more information:
// http://wikiri.upc.es/index.php/Robotics_Lab

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_H

#include "PPDDLParserInterface.h"

class PPDDLDeterminizator {
public: // FIXME static?
    virtual PPDDLInterface::Domain determinize(const PPDDLInterface::Domain& d); // FIXME non pure virt, take from MLO?
    virtual PPDDLInterface::ActionPtr determinize(const PPDDLInterface::Action&);// FIXME non pure virt
    virtual PPDDLInterface::EffectPtr determinize(const PPDDLInterface::Effect& e);
    virtual PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ConjunctiveEffect& ce) =0;
    virtual PPDDLInterface::EffectPtr determinize(const PPDDLInterface::ProbabilisticEffect& pe) =0;

    /*!
     * Creates a determinizator
     * @param method_suffix Suffix to the determinized domain name
     */
    PPDDLDeterminizator(std::string method_suffix="");
private:
    std::string _method_name_suffix;
};


#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLDETERMINIZATOR_H
