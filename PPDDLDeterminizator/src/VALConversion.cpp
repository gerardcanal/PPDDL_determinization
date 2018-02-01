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
    d.constants = new VAL::const_symbol_list; /* const_symbol_list* */
    std::vector<std::string> term_names = _dom->terms().names(); // Variable reuse!
    for (auto it = term_names.begin(); it != term_names.end(); ++it) {

        const ppddl_parser::Object* x = _dom->terms().find_object(*it); // If not in the domain, the terms are objects from the problem file
        if (x != nullptr) {
            VAL::const_symbol* cs = new VAL::const_symbol(*it);
            std::string t_name = _dom->types().typestring(_dom->terms().type(ppddl_parser::Term(*x)));
            cs->type = new VAL::pddl_type(t_name);
            d.constants->push_back(cs);
        }
    }

    d.pred_vars;  // Vars used in predicate declarations /* var_symbol_table* */
    // Not filling this one as it is not getting used by VAL.

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PREDICATES
    d.predicates = new VAL::pred_decl_list; /* pred_decl_list* */
    names = _dom->predicates().names();
    for (auto it = names.begin(); it != names.end(); ++it) {
        VAL::pred_symbol* s = new VAL::pred_symbol(*it);
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;
        VAL::var_symbol_table* st = new VAL::var_symbol_table();

        // get all the parameters/arguments
        ppddl_parser::TypeList tl = _dom->predicates().parameters(*_dom->predicates().find_predicate(*it));
        std::map<std::string, int> var_names;
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            // get type name
            std::string t_name = _dom->types().typestring(*tlit);

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_names.find(vname) == var_names.end()) var_names[vname] = 0;
            VAL::var_symbol* vs = new VAL::var_symbol(vname + ((var_names[vname] == 0)? "" : std::to_string(var_names[vname])));
            ++var_names[vname];

            // Define the type and set object
            vs->type = new VAL::pddl_type(t_name);
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }

        d.predicates->push_back(new VAL::pred_decl(s, sl, st));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // FUNCTIONS
    d.functions = new VAL::func_decl_list; /* func_decl_list* */
    names = _dom->functions().names();
    std::set<std::string> unique_names(names.begin(), names.end()); // Some times names() has duplicated function names, I make them unique by inserting to the set.
    for (auto it = unique_names.begin(); it != unique_names.end(); ++it) {
        VAL::func_symbol* fs = new VAL::func_symbol(*it);
        VAL::var_symbol_list* sl = new VAL::var_symbol_list;
        VAL::var_symbol_table* st = new VAL::var_symbol_table;

        const ppddl_parser::TypeList fparam = _dom->functions().parameters(*_dom->functions().find_function(*it));
        std::map<std::string, int> var_names;
        for (auto fparamit = fparam.begin(); fparamit != fparam.end(); ++fparamit) {
            // get type name
            std::string t_name = _dom->types().typestring(*fparamit);

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_names.find(vname) == var_names.end()) var_names[vname] = 0;
            VAL::var_symbol* vs = new VAL::var_symbol(vname + ((var_names[vname] == 0)? "" : std::to_string(var_names[vname])));
            ++var_names[vname];

            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }

        d.functions->push_back(new VAL::func_decl(fs, sl, st));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.constraints; /* TODO con_goal* -> not printed!? */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.ops = new VAL::operator_list; /* TODO operator_list* */

    // Add objects and constants as declared variables!
    std::map<ppddl_parser::Term, std::string> const_obj_decl;
    std::vector<std::string> obj_names = _dom->terms().names();
    for (auto oit = obj_names.begin(); oit != obj_names.end(); ++oit) {
        const ppddl_parser::Object* obj = _dom->terms().find_object(*oit);
        if (obj != nullptr) {
            const_obj_decl[*obj] = *oit;
        }
        else {
            for (auto probit = ppddl_parser::Problem::begin(); probit != ppddl_parser::Problem::end(); ++probit) {
                obj = probit->second->terms().find_object(*oit);
                if (obj != nullptr) {
                    const_obj_decl[*obj] = *oit;
                }
            }
        }
    }

    for (auto it = _dom->actions().begin(); it != _dom->actions().end(); ++it) {
        VAL::operator_symbol *name = new VAL::operator_symbol(it->first);
        VAL::var_symbol_table* symtab = new VAL::var_symbol_table();

        VAL::var_symbol_list* parameters = new VAL::var_symbol_list();
        std::map<std::string, int> var_name_ctr;
        std::map<ppddl_parser::Term, std::string> var_decl(const_obj_decl.begin(), const_obj_decl.end());
        for (auto pit = it->second->parameters().begin(); pit != it->second->parameters().end(); ++pit) {
            std::string t_name = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*pit));

            // Define variable name: first letter of the type. If more than one object of the same type, it will be i.e. f, f1, f2, f3...
            std::string vname = t_name.substr(0,1);
            if (var_name_ctr.find(vname) == var_name_ctr.end()) var_name_ctr[vname] = 0;
            std::string new_vname = vname + ((var_name_ctr[vname] == 0)? "" : std::to_string(var_name_ctr[vname]));
            VAL::var_symbol* vs = new VAL::var_symbol(new_vname);
            ++var_name_ctr[vname];
            var_decl[ppddl_parser::Term(*pit)] = new_vname; // Here I set the name to the term to use it accordingly

            // Define the type and set object
            vs->type = new VAL::pddl_type(t_name);
            parameters->push_back(vs);
            symtab->insert(std::make_pair(vname, vs));
        }

        VAL::goal *precondition = toVALCondition(&it->second->precondition(), _dom, var_name_ctr,var_decl);

        VAL::effect_lists* effects = toVALEffects(&it->second->effect(), _dom, var_name_ctr, var_decl);
        //
        VAL::operator_* op = new VAL::action(name, parameters, precondition, effects, symtab);
        d.ops->push_back(op);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.drvs; /* TODO derivations_list* */

    return d;
}


VAL::goal* VALConversion::toVALCondition(const ppddl_parser::StateFormula *precondition,
                                         const std::shared_ptr<ppddl_parser::Domain> &dom,
                                         std::map<std::string, int>& var_name_ctr,
                                         std::map<ppddl_parser::Term, std::string>& var_decl) {
    const ppddl_parser::Atom* a = dynamic_cast<const ppddl_parser::Atom*>(precondition);
    if (a != nullptr) {
        VAL::pred_symbol* h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list* sl = new VAL::parameter_symbol_list;
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
        return new VAL::comparison(VAL::comparison_op::E_LESS, toVALExpression(&cmplt->expr1(), dom),
                                                   toVALExpression(&cmplt->expr2(), dom));
    }

    const ppddl_parser::LessThanOrEqualTo* cmplte = dynamic_cast<const ppddl_parser::LessThanOrEqualTo*>(precondition);
    if (cmplte != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_LESSEQ, toVALExpression(&cmplte->expr1(), dom),
                                   toVALExpression(&cmplte->expr2(), dom));
    }

    const ppddl_parser::EqualTo* cmpeq = dynamic_cast<const ppddl_parser::EqualTo*>(precondition);
    if (cmpeq != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_EQUALS, toVALExpression(&cmpeq->expr1(), dom),
                                   toVALExpression(&cmpeq->expr2(), dom));
    }

    const ppddl_parser::GreaterThanOrEqualTo* cmpgte = dynamic_cast<const ppddl_parser::GreaterThanOrEqualTo*>(precondition);
    if (cmpgte != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_GREATEQ, toVALExpression(&cmpgte->expr1(), dom),
                                   toVALExpression(&cmpgte->expr2(), dom));
    }

    const ppddl_parser::GreaterThan* cmpgt = dynamic_cast<const ppddl_parser::GreaterThan*>(precondition);
    if (cmpgt != nullptr) {
        return new VAL::comparison(VAL::comparison_op::E_GREATER, toVALExpression(&cmpgt->expr1(), dom),
                                   toVALExpression(&cmpgt->expr2(), dom));
    }

    const ppddl_parser::Negation* n = dynamic_cast<const ppddl_parser::Negation*>(precondition);
    if (n != nullptr) {
        return new VAL::neg_goal(toVALCondition(&n->negand(), dom, var_name_ctr, var_decl));
    }

    const ppddl_parser::Conjunction* cjt = dynamic_cast<const ppddl_parser::Conjunction*>(precondition);
    if (cjt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList cjs = cjt->conjuncts();
        for (auto it = cjs.begin(); it != cjs.end(); ++it) {
            gl->push_back(toVALCondition(*it, dom, var_name_ctr, var_decl));
        }
        return new VAL::conj_goal(gl);
    }

    const ppddl_parser::Disjunction* djt = dynamic_cast<const ppddl_parser::Disjunction*>(precondition);
    if (djt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList djs = djt->disjuncts();
        for (auto it = djs.begin(); it != djs.end(); ++it) {
            gl->push_back(toVALCondition(*it, dom, var_name_ctr, var_decl));
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
            vs->type = new VAL::pddl_type(t_name);
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_EXISTS, sl, toVALCondition(&ex->body(), dom,var_name_ctr, var_decl), st);
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
            vs->type = new VAL::pddl_type(t_name);
            sl->push_back(vs);
            st->insert(std::make_pair(vname, vs));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_FORALL, sl, toVALCondition(&fa->body(), dom, var_name_ctr, var_decl), st);
    }

    throw std::runtime_error("Error: [toVALCondition] At least one condition should have been satisfied! Unrecognized StateFormula type.");
}

VAL::expression * VALConversion::toVALExpression(const ppddl_parser::Expression *exp,
                                                 const std::shared_ptr<ppddl_parser::Domain> &dom) {
    const ppddl_parser::Value* v = dynamic_cast<const ppddl_parser::Value*>(exp);
    if (v != 0) {
        return new VAL::float_expression(v->value().double_value());
    }

    const ppddl_parser::Fluent* f = dynamic_cast<const ppddl_parser::Fluent*>(exp);
    if (f != 0) {
        ppddl_parser::TermList tll = f->terms(); // FIXME what are these?
        VAL::func_symbol* fs = new VAL::func_symbol(dom->functions().name(f->function()));
        VAL::parameter_symbol_list* sl = new VAL::parameter_symbol_list();
        ppddl_parser::TypeList tl = dom->functions().parameters(f->function());
        for (auto it = tl.begin(); it != tl.end(); ++it ) {
            sl->push_back(new VAL::parameter_symbol(dom->types().typestring(*it)));
        }

        return new VAL::func_term(fs, sl);
    }

    const ppddl_parser::Addition* c = dynamic_cast<const ppddl_parser::Addition*>(exp);
    if (c != nullptr) {
        VAL::expression* a1 = toVALExpression(&c->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&c->operand2(), nullptr);
        return new VAL::plus_expression(a1, a2);
    }

    const ppddl_parser::Subtraction* s = dynamic_cast<const ppddl_parser::Subtraction*>(exp);
    if (s != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::minus_expression(a1, a2);
    }

    const ppddl_parser::Multiplication* m = dynamic_cast<const ppddl_parser::Multiplication*>(exp);
    if (m != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::mul_expression(a1, a2);
    }

    const ppddl_parser::Division* d = dynamic_cast<const ppddl_parser::Division*>(exp);
    if (d != nullptr) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::div_expression(a1, a2);
    }
    throw std::runtime_error("Error: [toVALExpression] At least one condition should have been satisfied! Unrecognized Expression type.");
}

VAL::effect_lists *VALConversion::toVALEffects(const ppddl_parser::Effect *e,
                                               const std::shared_ptr<ppddl_parser::Domain> &dom,
                                               std::map<std::string, int>& var_name_ctr,
                                               std::map<ppddl_parser::Term, std::string>& var_decl) {
    VAL::effect_lists* ef = new VAL::effect_lists();

    const ppddl_parser::SimpleEffect* se = dynamic_cast<const ppddl_parser::SimpleEffect*>(e);
    if (se != nullptr) {
        const ppddl_parser::Atom *a = &se->atom();

        VAL::pred_symbol *h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list *sl = new VAL::parameter_symbol_list;
        ppddl_parser::TermList tl = a->terms();
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            if (tlit->object()) sl->push_back(new VAL::const_symbol(var_decl[*tlit]));
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
        VAL::assignment* ass = toVALUpdate(&ue->update(), dom);
        ef->assign_effects.push_back(ass);
        return ef;
    }

    const ppddl_parser::ConjunctiveEffect* ce = dynamic_cast<const ppddl_parser::ConjunctiveEffect*>(e);
    if (ce != nullptr) {
        ppddl_parser::EffectList cjts = ce->conjuncts();
        for (auto it = cjts.begin(); it != cjts.end(); ++it) {
            VAL::effect_lists* conjunct = toVALEffects(*it, dom, var_name_ctr, var_decl);
            ef->append_effects(conjunct);
        }
        return ef;
    }

    const ppddl_parser::ConditionalEffect* cone = dynamic_cast<const ppddl_parser::ConditionalEffect*>(e);
    if (cone != nullptr) {
        VAL::cond_effect* condeff = new VAL::cond_effect(toVALCondition(&cone->condition(), dom, var_name_ctr, var_decl), toVALEffects(&cone->effect(), dom, var_name_ctr, var_decl));
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

        VAL::forall_effect* fae = new VAL::forall_effect(toVALEffects(&qe->effect(), dom, var_name_ctr, var_decl), sl, st);
        ef->forall_effects.push_back(fae);
        return ef;
    }

    const ppddl_parser::ProbabilisticEffect* pe = dynamic_cast<const ppddl_parser::ProbabilisticEffect*>(e);
    if (pe != nullptr) {
        throw std::runtime_error("Error: Probabilistic effects can not be converted to PDDL! Please determinize the domain first.");
    }
    throw std::runtime_error("Error: [toVALEffects] At least one condition should have been satisfied! Unrecognized Effect type.");
}

VAL::assignment *VALConversion::toVALUpdate(const ppddl_parser::Update *up, const std::shared_ptr<ppddl_parser::Domain> &dom) {

    VAL::func_term* ft = dynamic_cast<VAL::func_term*>(toVALExpression(&up->fluent(), dom)); // As we pass a Fluent, it will return the correct func_term
    VAL::expression* exp = toVALExpression(&up->expression(), dom);
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

