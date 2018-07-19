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
    /*!
     * Returns the wrapped VAL::domain object.
     * @return Weapped VAL::domain.
     */
    const std::shared_ptr<VAL::domain> get() { return _domain;}
    ~VALDomain() {};
private:
    /*!
     * Private constructor, as it shall only be created by the VALConversion class.
     * @param d Domain to be wrapped
     */
    explicit VALDomain(VAL::domain* d) : _domain(std::shared_ptr<VAL::domain>(d)) {};
    std::shared_ptr<VAL::domain> _domain; //!< Pointer to the wrapped domain
    friend class VALConversion;
};

/*!
 * \class VALProblem
 * \brief Wrapper class for the VAL::problem class. Can only be created through the VALConversion::toVALProblem() static method.
 */
class VALProblem : public VALWrapper {
    // Wrapper for a VAL::problem
public:
    /*!
     * Returns the wrapped VAL::problem object.
     * @return The wrapped object
     */
    const std::shared_ptr<VAL::problem> get() { return _problem;}
    ~VALProblem() {};
private:
    /*!
     * Private constructor, as it shall only be created by the VALConversion class.
     * @param p Problem to be wrapped
     */
    explicit VALProblem(VAL::problem* p) : _problem(p) {};
    std::shared_ptr<VAL::problem> _problem;
    friend class VALConversion;
};

/*!
 * \class VALConversion
 * \brief Class to convert from ppddl_parser structures to VAL PDDL structures. The input domain must be determinized
 * (i.e. can't contain Proababilistic effects).
 */
class VALConversion {
public:
    /*!
     * Converts a ppddl_parser::Domain to a VAL::domain.
     * @param dom Domain to be converted
     * @return The converted domain
     */
    static std::shared_ptr<VALDomain> toVALDomain(const ppddl_parser::Domain* dom);

    /*!
     * Converts a ppddl_parser::Problem to a VAL::problem.
     * @param p Problem to be converted.
     * @param domainwrap Wrapped VAL::domain of this problem
     * @return The converted problem
     */
    static std::shared_ptr<VALProblem> toVALProblem(const ppddl_parser::Problem *p,
                                                    const std::shared_ptr<VALDomain> domainwrap);
private:
    static VAL::goal *toVALCondition(const ppddl_parser::StateFormula *formula, const ppddl_parser::Domain *dom,
                                     std::map<std::string, int> &var_name_ctr,
                                     std::map<ppddl_parser::Term, std::string> &var_decl,
                                     std::shared_ptr<VALWrapper> valwrap);
    static VAL::expression *toVALExpression(const ppddl_parser::Expression *exp, const ppddl_parser::Domain *dom,
                                            std::map<ppddl_parser::Term, std::string> &var_decl,
                                            std::shared_ptr<VALWrapper> valwrap);
    static VAL::effect_lists *toVALEffects(const ppddl_parser::Effect *e, const ppddl_parser::Domain *dom,
                                           std::map<std::string, int> &var_name_ctr,
                                           std::map<ppddl_parser::Term, std::string> &var_decl,
                                           std::shared_ptr<VALWrapper> valwrap);

    static VAL::assignment *
    toVALUpdate(const ppddl_parser::Update *up, const ppddl_parser::Domain *dom,
                std::map<ppddl_parser::Term, std::string> &var_decl,std::shared_ptr<VALWrapper> valwrap);

    static VAL::pddl_req_flag toVALRequirements(const ppddl_parser::Requirements *req);
};


#endif //ROSPLAN_PLANNING_SYSTEM_VALCONVERSION_H
