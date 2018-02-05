//
// Created by gcanal on 29/01/18.
//

#include <cstring>
#include "VALConversion.h"

VALDomain* VALConversion::toVALDomain(const ppddl_parser::Domain* dom) {
    VAL::domain* d = new VAL::domain(new VAL::structure_store()); // The pointer is deleted inside the constructor
    VALDomain* _domain_wrapper = new VALDomain(d);

    d->name = dom->name();

    d->req = toVALRequirements(&dom->requirements); /* pddl_req_flag */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TYPES
    d->types = new VAL::pddl_type_list;
    std::vector<std::string> names = dom->types().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::pddl_type* t = new VAL::pddl_type(*it);
        d->types->push_back(t);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONSTANTS -- aka terms()
    d->constants = new VAL::const_symbol_list; /* const_symbol_list* */
    std::vector<std::string> term_names = dom->terms().names(); // Variable reuse!
    for (auto it = term_names.begin(); it != term_names.end(); ++it) {

        const ppddl_parser::Object* x = dom->terms().find_object(*it); // If not in the domain, the terms are objects from the problem file
        if (x != nullptr) {
            VAL::const_symbol* cs;
            auto constit = _domain_wrapper->const_tab.find(*it);
            if (constit == _domain_wrapper->const_tab.end()) { // Not found
                cs = new VAL::const_symbol(*it);
                std::string t_name = dom->types().typestring(dom->terms().type(ppddl_parser::Term(*x)));

                cs->type = _domain_wrapper->pddl_type_tab.find(t_name)->second;// FIXME? new VAL::pddl_type(t_name);
                _domain_wrapper->const_tab.insert(std::make_pair(*it, cs));
            }
            else cs = constit->second;
            d->constants->push_back(cs);
        }
    }

    d->pred_vars;  // Vars used in predicate declarations /* var_symbol_table* */
    // Not filling this one as it is not getting used by VAL.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PREDICATES
    d->predicates = new VAL::pred_decl_list; /* pred_decl_list* */
    names = dom->predicates().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::pred_symbol* s = new VAL::pred_symbol(*it);
        _domain_wrapper->pred_tab.insert(std::make_pair(*it, s));
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;
        VAL::var_symbol_table* st = new VAL::var_symbol_table();

        // get all the parameters/arguments
        ppddl_parser::TypeList tl = dom->predicates().parameters(*dom->predicates().find_predicate(*it));
        std::map<std::string, int> var_names;
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            // get type name
            std::string t_name = dom->types().typestring(*tlit);

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_names.find(vname) == var_names.end()) var_names[vname] = 0;
            VAL::var_symbol* vs = new VAL::var_symbol(vname + ((var_names[vname] == 0)? "" : std::to_string(var_names[vname])));
            ++var_names[vname];

            // Define the type and set object
            vs->type = _domain_wrapper->pddl_type_tab.find(t_name)->second; // Types were already defined
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }

        d->predicates->push_back(new VAL::pred_decl(s, sl, st));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    d->functions = new VAL::func_decl_list; /* func_decl_list* */
    names = dom->functions().names();
    std::set<std::string> unique_names(names.begin(), names.end()); // Some times names() has duplicated function names, I make them unique by inserting to the set.
    for (auto it = unique_names.begin(); it != unique_names.end(); ++it) {
        VAL::func_symbol* fs = new VAL::func_symbol(*it);
        _domain_wrapper->func_tab.insert(std::make_pair(*it, fs));
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;
        VAL::var_symbol_table* st = new VAL::var_symbol_table;

        const ppddl_parser::TypeList fparam = dom->functions().parameters(*dom->functions().find_function(*it));
        std::map<std::string, int> var_names;
        for (auto fparamit = fparam.begin(); fparamit != fparam.end(); ++fparamit) {
            // get type name
            std::string t_name = dom->types().typestring(*fparamit);

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_names.find(vname) == var_names.end()) var_names[vname] = 0;
            VAL::var_symbol* vs = new VAL::var_symbol(vname + ((var_names[vname] == 0)? "" : std::to_string(var_names[vname])));
            ++var_names[vname];

            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }

        d->functions->push_back(new VAL::func_decl(fs, sl, st));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d->constraints; /* con_goal* -> not printed!? */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d->ops = new VAL::operator_list; /* operator_list* */

    // Add objects and constants as declared variables!
    std::map<ppddl_parser::Term, std::string> const_obj_decl; // The name (string) of each term
    std::vector<std::string> obj_names = dom->terms().names();
    for (auto oit = obj_names.begin(); oit != obj_names.end(); ++oit) {
        const ppddl_parser::Object* obj = dom->terms().find_object(*oit); // For the domain
        if (obj != nullptr) {
            const_obj_decl[*obj] = *oit;
        }
        else { // For those defined in the problem file
            for (auto probit = ppddl_parser::Problem::begin(); probit != ppddl_parser::Problem::end(); ++probit) {
                obj = probit->second->terms().find_object(*oit);
                if (obj != nullptr) {
                    const_obj_decl[*obj] = *oit;
                }
            }
        }
    }

    for (auto it = dom->actions().begin(); it != dom->actions().end(); ++it) {
        VAL::operator_symbol *op_symbol = new VAL::operator_symbol(it->first);
        _domain_wrapper->op_tab.insert(std::make_pair(it->first, op_symbol));
        VAL::var_symbol_table* symtab = new VAL::var_symbol_table();

        VAL::var_symbol_list* parameters = new VAL::var_symbol_list();
        std::map<std::string, int> var_name_ctr;
        std::map<ppddl_parser::Term, std::string> var_decl(const_obj_decl.begin(), const_obj_decl.end());
        for (auto pit = it->second->parameters().begin(); pit != it->second->parameters().end(); ++pit) {
            std::string t_name = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*pit));

            // Define variable op_symbol: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_name_ctr.find(vname) == var_name_ctr.end()) var_name_ctr[vname] = 0;
            std::string new_vname = vname + ((var_name_ctr[vname] == 0)? "" : std::to_string(var_name_ctr[vname]));
            VAL::var_symbol* vs = new VAL::var_symbol(new_vname);
            ++var_name_ctr[vname];
            var_decl[ppddl_parser::Term(*pit)] = new_vname; // Here I set the op_symbol to the term to use it accordingly

            // Define the type and set object
            vs->type = _domain_wrapper->pddl_type_tab.find(t_name)->second;
            parameters->push_back(vs);
            symtab->insert(std::make_pair(vname, vs));
        }

        VAL::goal *precondition = toVALCondition(&it->second->precondition(), dom, var_name_ctr, var_decl,
                                                 _domain_wrapper);

        VAL::effect_lists* effects = toVALEffects(&it->second->effect(), dom, var_name_ctr, var_decl, _domain_wrapper);
        //
        VAL::action* op = new VAL::action(op_symbol, parameters, precondition, effects, symtab);
        d->ops->push_back(op);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // d->drvs; /* derivations_list* */ They are not printed in the pddl

    return _domain_wrapper;
}


VAL::goal * VALConversion::toVALCondition(const ppddl_parser::StateFormula *precondition,
                                          const ppddl_parser::Domain *dom,
                                          std::map<std::string, int> &var_name_ctr,
                                          std::map<ppddl_parser::Term, std::string> &var_decl,
                                          const VALWrapper *valwrap) {
    const ppddl_parser::Atom* a = dynamic_cast<const ppddl_parser::Atom*>(precondition);
    if (a != nullptr) {
        VAL::pred_symbol* h;
        auto predit = valwrap->pred_tab.find(ppddl_parser::PredicateTable::name(a->predicate()));
        if (predit == valwrap->pred_tab.end()) {
            h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        }
        else h = predit->second;

        VAL::parameter_symbol_list *sl = new VAL::parameter_symbol_list;
        ppddl_parser::TermList tl = a->terms();
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            if (tlit->object()) sl->push_back(new VAL::const_symbol(var_decl[*tlit]));
            else sl->push_back(new VAL::var_symbol(var_decl[*tlit]));
        }

        VAL::proposition* prop = new VAL::proposition(h, sl);
        return new VAL::simple_goal(prop, VAL::polarity::E_POS);
    }

    const ppddl_parser::Equality* eq = dynamic_cast<const ppddl_parser::Equality*>(precondition);
    if (eq != nullptr) {
        std::cerr << "Equality component could not be converted" << std::endl;
        return 0;
    }

    const ppddl_parser::LessThan* cmplt = dynamic_cast<const ppddl_parser::LessThan*>(precondition);
    if (cmplt != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_LESS, toVALExpression(&cmplt->expr1(), dom, valwrap),
                                   toVALExpression(&cmplt->expr2(), dom, valwrap));
    }

    const ppddl_parser::LessThanOrEqualTo* cmplte = dynamic_cast<const ppddl_parser::LessThanOrEqualTo*>(precondition);
    if (cmplte != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_LESSEQ, toVALExpression(&cmplte->expr1(), dom, valwrap),
                                   toVALExpression(&cmplte->expr2(), dom, valwrap));
    }

    const ppddl_parser::EqualTo* cmpeq = dynamic_cast<const ppddl_parser::EqualTo*>(precondition);
    if (cmpeq != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_EQUALS, toVALExpression(&cmpeq->expr1(), dom, valwrap),
                                   toVALExpression(&cmpeq->expr2(), dom, valwrap));
    }

    const ppddl_parser::GreaterThanOrEqualTo* cmpgte = dynamic_cast<const ppddl_parser::GreaterThanOrEqualTo*>(precondition);
    if (cmpgte != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_GREATEQ,
                                   toVALExpression(&cmpgte->expr1(), dom, valwrap),
                                   toVALExpression(&cmpgte->expr2(), dom, valwrap));
    }

    const ppddl_parser::GreaterThan* cmpgt = dynamic_cast<const ppddl_parser::GreaterThan*>(precondition);
    if (cmpgt != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_GREATER, toVALExpression(&cmpgt->expr1(), dom, valwrap),
                                   toVALExpression(&cmpgt->expr2(), dom, valwrap));
    }

    const ppddl_parser::Negation* n = dynamic_cast<const ppddl_parser::Negation*>(precondition);
    if (n != nullptr) {
        return new VAL::neg_goal(toVALCondition(&n->negand(), dom, var_name_ctr, var_decl, valwrap));
    }

    const ppddl_parser::Conjunction* cjt = dynamic_cast<const ppddl_parser::Conjunction*>(precondition);
    if (cjt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList cjs = cjt->conjuncts();
        for (auto it = cjs.begin(); it != cjs.end(); ++it) {
            gl->push_back(toVALCondition(*it, dom, var_name_ctr, var_decl, valwrap));
        }
        return new VAL::conj_goal(gl);
    }

    const ppddl_parser::Disjunction* djt = dynamic_cast<const ppddl_parser::Disjunction*>(precondition);
    if (djt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList djs = djt->disjuncts();
        for (auto it = djs.begin(); it != djs.end(); ++it) {
            gl->push_back(toVALCondition(*it, dom, var_name_ctr, var_decl, valwrap));
        }
        return new VAL::disj_goal(gl);
    }

    const ppddl_parser::Exists* ex = dynamic_cast<const ppddl_parser::Exists*>(precondition);
    if (ex != nullptr) {
        ppddl_parser::VariableList params = ex->parameters();
        VAL::var_symbol_list* sl = new VAL::var_symbol_list();
        VAL::var_symbol_table* st = new VAL::var_symbol_table();
        for (auto it = params.begin(); it != params.end(); ++it) {
            std::string t_name = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*it));

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_name_ctr.find(vname) == var_name_ctr.end()) var_name_ctr[vname] = 0;
            std::string new_vname = vname + ((var_name_ctr[vname] == 0)? "" : std::to_string(var_name_ctr[vname]));
            VAL::var_symbol* vs = new VAL::var_symbol(new_vname);
            ++var_name_ctr[vname];
            var_decl[ppddl_parser::Term(*it)] = new_vname; // Here I set the name to the term to use it accordingly

            // Define the type and set object
            vs->type = valwrap->pddl_type_tab.find(t_name)->second;
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_EXISTS, sl,
                                   toVALCondition(&ex->body(), dom, var_name_ctr, var_decl, valwrap), st);
    }

    const ppddl_parser::Forall* fa = dynamic_cast<const ppddl_parser::Forall*>(precondition);
    if (fa != nullptr) {
        ppddl_parser::VariableList params = fa->parameters();
        VAL::var_symbol_list* sl = new VAL::var_symbol_list();
        VAL::var_symbol_table* st = new VAL::var_symbol_table();
        for (auto it = params.begin(); it != params.end(); ++it) {
            std::string t_name = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*it));

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_name_ctr.find(vname) == var_name_ctr.end()) var_name_ctr[vname] = 0;
            std::string new_vname = vname + ((var_name_ctr[vname] == 0)? "" : std::to_string(var_name_ctr[vname]));
            VAL::var_symbol* vs = new VAL::var_symbol(new_vname);
            ++var_name_ctr[vname];
            var_decl[ppddl_parser::Term(*it)] = new_vname; // Here I set the name to the term to use it accordingly

            // Define the type and set object
            vs->type = valwrap->pddl_type_tab.find(t_name)->second;
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_FORALL, sl,
                                   toVALCondition(&fa->body(), dom, var_name_ctr, var_decl, valwrap), st);
    }

    throw std::runtime_error("Error: [toVALCondition] At least one condition should have been satisfied! Unrecognized StateFormula type.");
}

VAL::expression * VALConversion::toVALExpression(const ppddl_parser::Expression *exp, const ppddl_parser::Domain *dom,
                                                 const VALWrapper *valwrap) {
    const ppddl_parser::Value* v = dynamic_cast<const ppddl_parser::Value*>(exp);
    if (v != 0) {
        return new VAL::float_expression(v->value().double_value());
    }

    const ppddl_parser::Fluent* f = dynamic_cast<const ppddl_parser::Fluent*>(exp);
    if (f != 0) {
        // ppddl_parser::TermList tll = f->terms(); // FIXME  Not sure what are these or if I should use them

        VAL::func_symbol* fs = valwrap->func_tab.find(dom->functions().name(f->function()))->second; // FIXME? new VAL::func_symbol(dom->functions().name(f->function()));
        VAL::parameter_symbol_list* sl = new VAL::parameter_symbol_list();
        ppddl_parser::TypeList tl = dom->functions().parameters(f->function());
        for (auto it = tl.begin(); it != tl.end(); ++it ) {
            sl->push_back(new VAL::parameter_symbol(dom->types().typestring(*it)));
        }

        return new VAL::func_term(fs, sl);
    }

    const ppddl_parser::Addition* c = dynamic_cast<const ppddl_parser::Addition*>(exp);
    if (c != nullptr) {
        VAL::expression* a1 = toVALExpression(&c->operand1(), dom, valwrap);
        VAL::expression* a2 = toVALExpression(&c->operand2(), dom, valwrap);
        return new VAL::plus_expression(a1, a2);
    }

    const ppddl_parser::Subtraction* s = dynamic_cast<const ppddl_parser::Subtraction*>(exp);
    if (s != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), dom, valwrap);
        VAL::expression* a2 = toVALExpression(&s->operand2(), dom, valwrap);
        return new VAL::minus_expression(a1, a2);
    }

    const ppddl_parser::Multiplication* m = dynamic_cast<const ppddl_parser::Multiplication*>(exp);
    if (m != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), dom, valwrap);
        VAL::expression* a2 = toVALExpression(&s->operand2(), dom, valwrap);
        return new VAL::mul_expression(a1, a2);
    }

    const ppddl_parser::Division* d = dynamic_cast<const ppddl_parser::Division*>(exp);
    if (d != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), dom, valwrap);
        VAL::expression* a2 = toVALExpression(&s->operand2(), dom, valwrap);
        return new VAL::div_expression(a1, a2);
    }
    throw std::runtime_error("Error: [toVALExpression] At least one condition should have been satisfied! Unrecognized Expression type.");
}

VAL::effect_lists *VALConversion::toVALEffects(const ppddl_parser::Effect *e, const ppddl_parser::Domain *dom,
                                               std::map<std::string, int> &var_name_ctr,
                                               std::map<ppddl_parser::Term, std::string> &var_decl,
                                               const VALWrapper *valwrap) {
    VAL::effect_lists* ef = new VAL::effect_lists();

    const ppddl_parser::SimpleEffect* se = dynamic_cast<const ppddl_parser::SimpleEffect*>(e);
    if (se != nullptr) {
        const ppddl_parser::Atom *a = &se->atom();

        VAL::pred_symbol *h = valwrap->pred_tab.find(ppddl_parser::PredicateTable::name(a->predicate()))->second;// FIXME? new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list *sl = new VAL::parameter_symbol_list;
        ppddl_parser::TermList tl = a->terms();
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            if (tlit->object()) sl->push_back(valwrap->const_tab.find(var_decl[*tlit])->second); // FIXME? new VAL::const_symbol(var_decl[*tlit]));
            else sl->push_back(new VAL::var_symbol(var_decl[*tlit]));
        }

        VAL::proposition *prop = new VAL::proposition(h, sl);

        const ppddl_parser::AddEffect* ae = dynamic_cast<const ppddl_parser::AddEffect*>(e);
        const ppddl_parser::DeleteEffect *de = dynamic_cast<const ppddl_parser::DeleteEffect *>(e);
        if (ae != nullptr) {
            ef->add_effects.push_back(new VAL::simple_effect(prop));
        }
        else if (de != 0) {
            ef->del_effects.push_back(new VAL::simple_effect(prop));
        }
        return ef;
    }

    const ppddl_parser::UpdateEffect* ue = dynamic_cast<const ppddl_parser::UpdateEffect*>(e); // ie decrease / increase
    if (ue != nullptr) {
        VAL::assignment* ass = toVALUpdate(&ue->update(), dom, valwrap);
        ef->assign_effects.push_back(ass);
        return ef;
    }

    const ppddl_parser::ConjunctiveEffect* ce = dynamic_cast<const ppddl_parser::ConjunctiveEffect*>(e);
    if (ce != nullptr) {
        ppddl_parser::EffectList cjts = ce->conjuncts();
        for (auto it = cjts.begin(); it != cjts.end(); ++it) {
            VAL::effect_lists* conjunct = toVALEffects(*it, dom, var_name_ctr, var_decl, valwrap);
            ef->append_effects(conjunct);
            delete conjunct; // As append_effects makes a copy, the pointer gets lost causing a memory leak
        }
        return ef;
    }

    const ppddl_parser::ConditionalEffect* cone = dynamic_cast<const ppddl_parser::ConditionalEffect*>(e);
    if (cone != nullptr) {
        VAL::cond_effect* condeff = new VAL::cond_effect(
                toVALCondition(&cone->condition(), dom, var_name_ctr, var_decl, valwrap),
                toVALEffects(&cone->effect(), dom, var_name_ctr, var_decl, valwrap));
        ef->cond_effects.push_back(condeff);
        return ef;
    }

    const ppddl_parser::QuantifiedEffect* qe = dynamic_cast<const ppddl_parser::QuantifiedEffect*>(e);
    if (qe != nullptr) {
        ppddl_parser::VariableList params = qe->parameters();
        VAL::var_symbol_list* sl = new VAL::var_symbol_list();
        VAL::var_symbol_table* st = new VAL::var_symbol_table();
        for (auto it = params.begin(); it != params.end(); ++it) {
            std::string t_name = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*it));

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_name_ctr.find(vname) == var_name_ctr.end()) var_name_ctr[vname] = 0;
            std::string new_vname = vname + ((var_name_ctr[vname] == 0)? "" : std::to_string(var_name_ctr[vname]));
            VAL::var_symbol* vs = new VAL::var_symbol(new_vname);
            ++var_name_ctr[vname];
            var_decl[ppddl_parser::Term(*it)] = new_vname; // Here I set the name to the term to use it accordingly

            // Define the type and set object
            vs->type = new VAL::pddl_type(t_name);
            sl->push_back(vs);
            st->insert(std::make_pair(t_name, vs));
        }

        VAL::forall_effect* fae = new VAL::forall_effect(
                toVALEffects(&qe->effect(), dom, var_name_ctr, var_decl, valwrap), sl, st);
        ef->forall_effects.push_back(fae);
        return ef;
    }

    const ppddl_parser::ProbabilisticEffect* pe = dynamic_cast<const ppddl_parser::ProbabilisticEffect*>(e);
    if (pe != nullptr) {
        throw std::runtime_error("Error: Probabilistic effects can not be converted to PDDL! Please determinize the domain first.");
    }
    throw std::runtime_error("Error: [toVALEffects] At least one condition should have been satisfied! Unrecognized Effect type.");
}

VAL::assignment *VALConversion::toVALUpdate(const ppddl_parser::Update *up, const ppddl_parser::Domain *dom, const VALWrapper *valwrap) {

    VAL::func_term* ft = dynamic_cast<VAL::func_term*>(toVALExpression(&up->fluent(), dom, valwrap)); // As we pass a Fluent, it will return the correct func_term
    VAL::expression* exp = toVALExpression(&up->expression(), dom, valwrap);
    VAL::assign_op op;

    if (dynamic_cast<const ppddl_parser::Assign*>(up) != nullptr) {
        op = VAL::assign_op::E_ASSIGN;
    }
    else if (dynamic_cast<const ppddl_parser::ScaleUp*>(up) != nullptr) {
        op = VAL::assign_op::E_SCALE_UP;
    }
    else if (dynamic_cast<const ppddl_parser::ScaleDown*>(up) != nullptr) {
        op = VAL::assign_op::E_SCALE_DOWN;
    }
    else if (dynamic_cast<const ppddl_parser::Increase*>(up) != nullptr) {
        op = VAL::assign_op::E_INCREASE;
    }
    else if (dynamic_cast<const ppddl_parser::Decrease*>(up) != nullptr) {
        op = VAL::assign_op::E_DECREASE;
    }
    else {
        throw std::runtime_error(
                "Error: [toVALUpdate] At least one condition should have been satisfied! Unrecognized Update type.");
    }
    return new VAL::assignment(ft, op, exp);
}

VALProblem VALConversion::toVALProblem(const ppddl_parser::Problem *p) {
    VAL::problem* problem = new VAL::problem();
    VALProblem ret(problem);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PROBLEM NAME
    problem->name = new char[p->name().size()+1];
    strcpy(problem->name, p->name().c_str());

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DOMAIN NAME
    problem->domain_name =  new char[p->domain().name().size()+1];
    strcpy(problem->domain_name, p->domain().name().c_str());

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PDDL REQUIREMENTS
    problem->req = toVALRequirements(&p->domain().requirements);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PDDL TYPES
    problem->types = new VAL::pddl_type_list;; //pddl_type_list*
    ppddl_parser::TypeTable ttable = p->domain().types();
    for (size_t i = 0; i < ttable.names().size(); ++i) {
        VAL::pddl_type* t = new VAL::pddl_type(ttable.names()[i]);
        problem->types->push_back(t);
        ret.pddl_type_tab.insert(std::make_pair(ttable.names()[i], t));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // OBJECTS
    problem->objects = new VAL::const_symbol_list; // const_symbol_list*

    map<std::string, int> var_name_ctr; // Not used but needed in the VALConversion methods
    std::map<ppddl_parser::Term, std::string> const_obj_decl;
    std::vector<std::string> obj_names = p->terms().names();
    for (auto oit = obj_names.begin(); oit != obj_names.end(); ++oit) {
        const ppddl_parser::Object* obj = p->terms().find_object(*oit);
        if (obj != nullptr) {
            const_obj_decl[*obj] = *oit;
            VAL::const_symbol* sym = new VAL::const_symbol(*oit);
            sym->type =  ret.pddl_type_tab.find(ttable.typestring(p->terms().type(ppddl_parser::Term(*obj))))->second; // FIXME? new VAL::pddl_type(ttable.typestring(p->terms().type(ppddl_parser::Term(*obj))));
            problem->objects->push_back(sym);
            ret.const_tab.insert(std::make_pair(*oit, sym));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // INITIAL STATE
    problem->initial_state = new VAL::effect_lists; /*effect_lists*/
    for (auto it = p->init_atoms().begin(); it != p->init_atoms().end();++it) {
        const ppddl_parser::Atom *a = *it;
        VAL::pred_symbol *h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list *sl = new VAL::parameter_symbol_list;
        ppddl_parser::TermList tl = a->terms();
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            if (tlit->object()) ret.const_tab.find(const_obj_decl[*tlit])->second;//FIXME? sl->push_back(new VAL::const_symbol(const_obj_decl[*tlit]));
            else sl->push_back(new VAL::var_symbol(const_obj_decl[*tlit]));
        }
        problem->initial_state->add_effects.push_back(new VAL::simple_effect(new VAL::proposition(h, sl)));
        ret.pred_tab.insert(std::make_pair(ppddl_parser::PredicateTable::name(a->predicate()), h));
    }

    // Add the rest
    for (auto eit = p->init_effects().begin(); eit != p->init_effects().end(); ++eit) {
        VAL::effect_lists* init_eff = toVALEffects(*eit, &p->domain(), var_name_ctr, const_obj_decl, &ret);
        problem->initial_state->append_effects(init_eff);
        delete init_eff; // As we appended them to the problem, they were copied and the pointer is lost
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GOAL
    problem->the_goal = toVALCondition(&p->goal(), &p->domain(), var_name_ctr, const_obj_decl, &ret); // goal*

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONSTRAINTS
    //problem->constraints;//con_goal * can't find something similar in the ppddl_parser GOAL REWARD??

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // METRIC
    VAL::optimization op = VAL::optimization::E_MAXIMIZE;
    const ppddl_parser::Expression* metric = &p->metric();
    const ppddl_parser::Subtraction* sub = dynamic_cast<const ppddl_parser::Subtraction*>(metric);
    if (sub != nullptr) {
        // The ppddl_parser always maximizes, and represents the minimization of X as maximize (- 0 X), so we check if it's a 0-X case
        const ppddl_parser::Value* op1 = dynamic_cast<const ppddl_parser::Value*>(&sub->operand1());
        if (op1 != nullptr && op1->value().double_value()==0) {
            // It is a 0-X case, so the metric is minimizing, and the value is the operand2.
            op = VAL::optimization::E_MINIMIZE;
            metric = &sub->operand2();
        }

    }
    problem->metric = new VAL::metric_spec(op, toVALExpression(metric, &p->domain(), &ret)); // metric_spec*

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LENGTH
    // problem->length; //length_spec* // Not available in ppddl_parser

    // ValueMap ppddl_parser::Problem::init_values() // It is not being used!
    // const Update *goal_reward_;
    return ret;
}

VAL::pddl_req_flag VALConversion::toVALRequirements(const ppddl_parser::Requirements *req) {
    VAL::pddl_req_flag reqflag = 0;

    if (req->strips) reqflag ^= VAL::pddl_req_attr::E_STRIPS;
    else if (req->typing) reqflag ^= VAL::pddl_req_attr::E_TYPING;
    else if (req->negative_preconditions) reqflag ^= VAL::pddl_req_attr::E_NEGATIVE_PRECONDITIONS;
    else if (req->disjunctive_preconditions) reqflag ^= VAL::pddl_req_attr::E_DISJUNCTIVE_PRECONDS;
    else if (req->equality) reqflag ^= VAL::pddl_req_attr::E_EQUALITY;
    else if (req->existential_preconditions) reqflag ^= VAL::pddl_req_attr::E_EXT_PRECS;
    else if (req->universal_preconditions) reqflag ^= VAL::pddl_req_attr::E_UNIV_PRECS;
    else if (req->conditional_effects) reqflag ^= VAL::pddl_req_attr::E_COND_EFFS;
    else if (req->fluents) reqflag ^= VAL::pddl_req_attr::E_NFLUENTS ^ VAL::pddl_req_attr::E_OFLUENTS;
    //else if (req->probabilistic_effects) // Can't be possible in a deterministic domain!
    else if (req->rewards) reqflag ^= VAL::pddl_req_attr::E_ACTIONCOSTS; // not sure if equivalent

    return reqflag;
}

