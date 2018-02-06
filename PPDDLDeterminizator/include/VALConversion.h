//
// Created by gcanal on 29/01/18.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
#define ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H


#include "PPDDLParser/domains.h"
#include "PPDDLParser/problems.h"
#include "ptree.h"

/*!
 * \class VALWrapper
 * \brief Generic wrapper class for the VAL structures. It stores symbol tables to keep the pointer values and delete them in the correct way.
 */
class VALWrapper { // handles the symbol tables so it ensures their deletion, preventing memory leaks
public:
    virtual ~VALWrapper() {
        for (auto it = var_list.begin(); it!=var_list.end(); ++it) delete *it;
    };
    VALWrapper() = default;
protected:
    VAL::const_symbol_table     const_tab; //!< Constant symbols table
    VAL::var_symbol_list        var_list; //!< Variable symbols list (it stores all the variables defined)
    VAL::pddl_type_symbol_table pddl_type_tab; //!< PDDL types table
    VAL::pred_symbol_table	    pred_tab; //!< Predicate symbols table
    VAL::func_symbol_table      func_tab; //!< Function symbols table
    VAL::operator_symbol_table  op_tab; //!< Operators -actions- symbols table
    friend class VALConversion;
};

/*!
 * \class VALDomain
 * \brief Wrapper class for the VAL::domain class. Can only be created through the VALConversion::toVALDomain() static method.
 */
class VALDomain : public VALWrapper {
    // Wrapper for a VAL::domain
public:
    const VAL::domain* get() { return _domain;}
    ~VALDomain() { delete _domain; };
private:
    explicit VALDomain(const VAL::domain* d) : _domain(d) {};
    const VAL::domain* _domain;
    friend class VALConversion;
};

/*!
 * \class VALProblem
 * \brief Wrapper class for the VAL::problem class. Can only be created through the VALConversion::toVALProblem() static method.
 */
class VALProblem : public VALWrapper {
    // Wrapper for a VAL::problem
public:
    const VAL::problem* get() {return _problem;}
    ~VALProblem() { delete _problem; };
private:
    explicit VALProblem(VAL::problem* p) : _problem(p) {};
    const VAL::problem* _problem;
    friend class VALConversion;
};

/*!
 * \class VALConversion
 * \brief Class to convert from ppddl_parser structures to VAL PDDL structures. The input domain must be determinized
 * (i.e. can't contain Proababilistic effects).
 */
class VALConversion {
public:
    static std::shared_ptr<VALDomain> toVALDomain(const ppddl_parser::Domain* dom);
    static std::shared_ptr<VALProblem> toVALProblem(const ppddl_parser::Problem *p,
                                                    const std::shared_ptr<VALDomain> domainwrap);
private:
    static VAL::goal *toVALCondition(const ppddl_parser::StateFormula *formula, const ppddl_parser::Domain *dom,
                                     std::map<std::string, int> &var_name_ctr,
                                     std::map<ppddl_parser::Term, std::string> &var_decl,
                                     std::shared_ptr<VALWrapper> valwrap);
    static VAL::expression *toVALExpression(const ppddl_parser::Expression *exp, const ppddl_parser::Domain *dom,
                                            std::shared_ptr<VALWrapper> valwrap);
    static VAL::effect_lists *toVALEffects(const ppddl_parser::Effect *e, const ppddl_parser::Domain *dom,
                                           std::map<std::string, int> &var_name_ctr,
                                           std::map<ppddl_parser::Term, std::string> &var_decl,
                                           std::shared_ptr<VALWrapper> valwrap);

    static VAL::assignment *
    toVALUpdate(const ppddl_parser::Update *up, const ppddl_parser::Domain *dom, std::shared_ptr<VALWrapper> valwrap);

    static VAL::pddl_req_flag toVALRequirements(const ppddl_parser::Requirements *req);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
