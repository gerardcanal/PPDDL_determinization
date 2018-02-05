//
// Created by gcanal on 29/01/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
#define ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H


#include "PPDDLParserInterface.h"
#include "ptree.h"

class VALWrapper { // handles the symbol tables so it ensures their deletion, preventing memory leaks
public:
    virtual ~VALWrapper() {
        for (auto it = var_list.begin(); it!=var_list.end(); ++it) delete *it;
    };
    VALWrapper() = default;
protected:
    VAL::const_symbol_table     const_tab;
    VAL::var_symbol_list        var_list;
    VAL::pddl_type_symbol_table pddl_type_tab;
    VAL::pred_symbol_table	    pred_tab;
    VAL::func_symbol_table      func_tab;
    VAL::operator_symbol_table  op_tab;
    friend class VALConversion;
};

class VALDomain : VALWrapper {
    // Wrapper for a VALDomain
public:
    const VAL::domain* get() {return _domain;}
    ~VALDomain() { delete _domain; };
private:
    explicit VALDomain(const VAL::domain* d) : _domain(d) {};
    const VAL::domain* _domain;
    friend class VALConversion;
};

class VALProblem : VALWrapper {
    // Wrapper for a VALDomain
public:
    const VAL::problem* get() {return _problem;}
    ~VALProblem() { delete _problem; };
private:
    explicit VALProblem(VAL::problem* p) : _problem(p) {};
    const VAL::problem* _problem;
    friend class VALConversion;
};

class VALConversion {
public:
    static VALDomain* toVALDomain(const ppddl_parser::Domain* dom);
    static VALProblem toVALProblem(const ppddl_parser::Problem *p, const VALDomain *domainwrap);
private:
    static VAL::goal *toVALCondition(const ppddl_parser::StateFormula *formula, const ppddl_parser::Domain *dom,
                                     std::map<std::string, int> &var_name_ctr,
                                     std::map<ppddl_parser::Term, std::string> &var_decl,
                                     VALWrapper *valwrap);
    static VAL::expression *toVALExpression(const ppddl_parser::Expression *exp, const ppddl_parser::Domain *dom,
                                            VALWrapper *valwrap);
    static VAL::effect_lists *toVALEffects(const ppddl_parser::Effect *e, const ppddl_parser::Domain *dom,
                                           std::map<std::string, int> &var_name_ctr,
                                           std::map<ppddl_parser::Term, std::string> &var_decl,
                                           VALWrapper *valwrap);

    static VAL::assignment *
    toVALUpdate(const ppddl_parser::Update *up, const ppddl_parser::Domain *dom, VALWrapper *valwrap);

    static VAL::pddl_req_flag toVALRequirements(const ppddl_parser::Requirements *req);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
