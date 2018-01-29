//
// Created by gcanal on 29/01/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
#define ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H


#include "PPDDLParserInterface.h"
#include "ptree.h"

class VALConversion {
public:
    static VAL::domain toVALDomain(const std::shared_ptr<ppddl_parser::Domain>& d);
private:
    static VAL::goal *
    toVALPrecondition(const ppddl_parser::StateFormula *precondition, const ppddl_parser::Domain *dom);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
