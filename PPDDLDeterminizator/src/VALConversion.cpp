//
// Created by gcanal on 29/01/18.
//

#include "VALConversion.h"

VAL::domain VALConversion::toVALDomain(const std::shared_ptr<ppddl_parser::Domain> &_dom) {
    VAL::domain d(new VAL::structure_store()); // The pointer is deleted inside the constructor
    d.name = _dom->name();

    d.req; /* TODO pddl_req_flag */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TYPES
    d.types = new VAL::pddl_type_list;
    std::vector<std::string> names = _dom->types().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::pddl_type* t = new VAL::pddl_type(*it);
        d.types->push_back(t); // FIXME it sets as symbols. Make sure the type is correct (as displays NULL)...
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONSTANTS -- aka terms()
    d.constants = new VAL::const_symbol_list; /* TODO const_symbol_list* */
    std::cout <<  _dom->terms().names().size() << " " << _dom->terms().names()[0] << std::endl;
    std::vector<std::string> term_names = _dom->terms().names(); // Variable reuse!
    for (auto it = term_names.begin(); it != term_names.end(); ++it) {
        std::cout << *it << std::endl;
        VAL::const_symbol* cs = new VAL::const_symbol(*it);

        const ppddl_parser::Object* x = _dom->terms().find_object(*it); // FIXME IF NOT IN DOMAIN, TERMS ARE IN THE PROBLEM FILE! TAKE THEM FROM THERE?
        if (x != nullptr)  _dom->terms().type(ppddl_parser::Term(*x));// TODO get type and finish this
        cs->type = new VAL::pddl_type(*it); // TODO PUT TYPE
        d.constants->push_back(cs);
    }

    d.pred_vars;  // Vars used in predicate declarations /* TODO var_symbol_table* */
    // TODO Not sure what this is so not filling this one... check if needed

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PREDICATES
    d.predicates = new VAL::pred_decl_list; /* pred_decl_list* */
    names = _dom->predicates().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::pred_symbol* s = new VAL::pred_symbol(*it);
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;

        // get all the parameters/arguments
        ppddl_parser::TypeList tl = _dom->predicates().parameters(*_dom->predicates().find_predicate(*it));
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            // get type name
            std::string t_name = _dom->types().typestring(*tlit);


            sl->push_back(new VAL::var_symbol(t_name));
        }

        d.predicates->push_back(new VAL::pred_decl(s, sl, new VAL::var_symbol_table));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    d.functions = new VAL::func_decl_list; /* func_decl_list* */
    names = _dom->functions().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::func_symbol* fs = new VAL::func_symbol(*it);
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;

        const ppddl_parser::TypeList fparam = _dom->functions().parameters(*_dom->functions().find_function(*it));
        for (auto fparamit = fparam.begin(); fparamit != fparam.end(); ++fparamit) {
            // get type name
            std::string t_name = _dom->types().typestring(*fparamit);

            sl->push_back(new VAL::var_symbol(t_name));
        }

        d.functions->push_back(new VAL::func_decl(fs, sl, new VAL::var_symbol_table));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.constraints; /* TODO con_goal* -> not printed */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.ops = new VAL::operator_list; /* TODO operator_list* */
    for (auto it = _dom->actions().begin(); it != _dom->actions().end(); ++it) {

        // TO fill
        VAL::operator_symbol *name = new VAL::operator_symbol(it->first);
        //fixme var_symbol_table *symtab;

        VAL::var_symbol_list *parameters = new VAL::var_symbol_list();
        for (auto pit = it->second->parameters().begin(); pit != it->second->parameters().end(); ++pit) {
            std::string pname;//fixme = term_names[pit];
            parameters->push_back(new VAL::var_symbol(pname));
        }

        VAL::goal *precondition = toVALPrecondition(&it->second->precondition(), nullptr);

        //effect_lists *effects;
        //
        //VAL::operator_* op = new VAL::operator_(aaa);
        //d.ops->push_back(op);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.drvs; /* TODO derivations_list* */

    return d;
}


VAL::goal* VALConversion::toVALPrecondition(const ppddl_parser::StateFormula *precondition, const ppddl_parser::Domain *dom) {
    const ppddl_parser::Atom* a = dynamic_cast<const ppddl_parser::Atom*>(precondition);
    if (a != nullptr) {
        VAL::pred_symbol* h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list* sl = new VAL::parameter_symbol_list;
        ppddl_parser::TypeList tl = ppddl_parser::PredicateTable::parameters(a->predicate());
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            // get type name
            std::string t_name = dom->types().typestring(*tlit);

            sl->push_back(new VAL::var_symbol(t_name));
        }

        VAL::proposition* prop = new VAL::proposition(h, sl);
        return new VAL::simple_goal(prop, VAL::polarity::E_POS);
    }

    const ppddl_parser::Equality* eq = dynamic_cast<const ppddl_parser::Equality*>(precondition);
    if (eq != nullptr) {
        return;
    }


    /*const ppddl_parser::Comparison* cmp = dynamic_cast<const ppddl_parser::Comparison*>(precondition);
    if (cmp != nullptr) {
        return; VAL::comparison
    }

    const ppddl_parser::Negation* n = dynamic_cast<const ppddl_parser::Negation*>(precondition);
    if (n != nullptr) {
        return; VAL::neg_goal
    }

    const ppddl_parser::Conjunction* cjt = dynamic_cast<const ppddl_parser::Conjunction*>(precondition);
    if (cjt != nullptr) {
        return; VAL::conj_goal
    }

    const ppddl_parser::Disjunction* djt = dynamic_cast<const ppddl_parser::Disjunction*>(precondition);
    if (djt != nullptr) {
        return; VAL::disj_goal
    }

    const ppddl_parser::Quantification* q = dynamic_cast<const ppddl_parser::Quantification*>(precondition);
    if (q != nullptr) {
        return; VAL::qfied_goal?
    }

    const ppddl_parser::Exists* ex = dynamic_cast<const ppddl_parser::Exists*>(precondition);
    if (ex != nullptr) {
        return;
    }

    const ppddl_parser::Forall* fa = dynamic_cast<const ppddl_parser::Forall*>(precondition);
    if (fa != nullptr) {
        return;
    }

    std::runtime_error("Unrecognized StateFormula type!");
    return new VAL::goal();*/
}

