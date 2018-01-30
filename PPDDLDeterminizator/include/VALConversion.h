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
    static VAL::goal* toVALPrecondition(const ppddl_parser::StateFormula *formula,
                                        const std::shared_ptr<ppddl_parser::Domain> &dom);
    static VAL::expression *toVALExpression(const ppddl_parser::Expression *exp,
                                            const std::shared_ptr<ppddl_parser::Domain> &dom);
    static VAL::effect_lists* toVALEffects(const ppddl_parser::Effect *e,
                                           const std::shared_ptr<ppddl_parser::Domain> &dom);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
