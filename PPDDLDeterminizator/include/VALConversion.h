//
// Created by gcanal on 29/01/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
#define ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H


#include "PPDDLParserInterface.h"
#include "ptree.h"

class VALConversion {
public:
    static VAL::domain toVALDomain(const ppddl_parser::Domain* dom);
    static VAL::problem toVALProblem(const ppddl_parser::Problem* p);
private:
    static VAL::goal* toVALCondition(const ppddl_parser::StateFormula *formula,
                                     const ppddl_parser::Domain* dom,
                                     std::map<std::string, int>& var_name_ctr,
                                     std::map<ppddl_parser::Term, std::string>& var_decl);
    static VAL::expression *toVALExpression(const ppddl_parser::Expression *exp,
                                            const ppddl_parser::Domain* dom);
    static VAL::effect_lists* toVALEffects(const ppddl_parser::Effect *e,
                                           const ppddl_parser::Domain* dom,
                                           std::map<std::string, int>& var_name_ctr,
                                           std::map<ppddl_parser::Term, std::string>& var_decl);

    static VAL::assignment *toVALUpdate(const ppddl_parser::Update *up, const ppddl_parser::Domain* dom);
    static VAL::pddl_req_flag toVALRequirements(const ppddl_parser::Requirements *req);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
