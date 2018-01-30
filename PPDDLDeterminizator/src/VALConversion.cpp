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
    d.constraints; /* TODO con_goal* -> not printed!? */

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.ops = new VAL::operator_list; /* TODO operator_list* */
    for (auto it = _dom->actions().begin(); it != _dom->actions().end(); ++it) {

        // TO fill
        VAL::operator_symbol *name = new VAL::operator_symbol(it->first);
        VAL::var_symbol_table* symtab = new VAL::var_symbol_table();

        VAL::var_symbol_list *parameters = new VAL::var_symbol_list();
        for (auto pit = it->second->parameters().begin(); pit != it->second->parameters().end(); ++pit) {
            std::string pname = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*pit));
            VAL::var_symbol* sym = new VAL::var_symbol(pname);
            parameters->push_back(sym);
            symtab->insert(std::make_pair(pname, sym));
        }

        VAL::goal *precondition = toVALPrecondition(&it->second->precondition(), _dom);

        VAL::effect_lists* effects = toVALEffects(&it->second->effect(), _dom);
        //
        VAL::operator_* op = new VAL::operator_(name, parameters, precondition, effects, symtab);
        d.ops->push_back(op);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    d.drvs; /* TODO derivations_list* */

    return d;
}


VAL::goal* VALConversion::toVALPrecondition(const ppddl_parser::StateFormula *precondition,
                                            const std::shared_ptr<ppddl_parser::Domain> &dom) {
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
        /*VAL::expression* exp1 = new VAL::expression();
        //VAL::comparison* cmp = new VAL::comparison(VAL::comparison_op::E_EQUALS, )*/
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
        return new VAL::neg_goal(toVALPrecondition(&n->negand(), dom));
    }

    const ppddl_parser::Conjunction* cjt = dynamic_cast<const ppddl_parser::Conjunction*>(precondition);
    if (cjt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList cjs = cjt->conjuncts();
        for (auto it = cjs.begin(); it != cjs.end(); ++it) {
            gl->push_back(toVALPrecondition(*it, dom));
        }
        return new VAL::conj_goal(gl);
    }

    const ppddl_parser::Disjunction* djt = dynamic_cast<const ppddl_parser::Disjunction*>(precondition);
    if (djt != nullptr) {
        VAL::goal_list* gl = new VAL::goal_list();
        ppddl_parser::FormulaList djs = djt->disjuncts();
        for (auto it = djs.begin(); it != djs.end(); ++it) {
            gl->push_back(toVALPrecondition(*it, dom));
        }
        return new VAL::disj_goal(gl);
    }

    const ppddl_parser::Exists* ex = dynamic_cast<const ppddl_parser::Exists*>(precondition);
    if (ex != nullptr) {
        ppddl_parser::VariableList params = ex->parameters();
        VAL::var_symbol_list* sl = new VAL::var_symbol_list();
        VAL::var_symbol_table* st = new VAL::var_symbol_table();
        for (auto it = params.begin(); it != params.end(); ++it) {
            std::string var = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*it));
            VAL::var_symbol* sym = new VAL::var_symbol(var);
            sl->push_back(sym);
            st->insert(std::make_pair(var, sym));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_EXISTS, sl, toVALPrecondition(&ex->body(), dom), st);
    }

    const ppddl_parser::Forall* fa = dynamic_cast<const ppddl_parser::Forall*>(precondition);
    if (fa != nullptr) {
        ppddl_parser::VariableList params = fa->parameters();
        VAL::var_symbol_list* sl = new VAL::var_symbol_list();
        VAL::var_symbol_table* st = new VAL::var_symbol_table();
        for (auto it = params.begin(); it != params.end(); ++it) {
            std::string var = ppddl_parser::TypeTable::typestring(ppddl_parser::TermTable::type(*it));
            VAL::var_symbol* sym = new VAL::var_symbol(var);
            sl->push_back(sym);
            st->insert(std::make_pair(var, sym));
        }
        return new VAL::qfied_goal(VAL::quantifier::E_FORALL, sl, toVALPrecondition(&fa->body(), dom), st);
    }

    std::runtime_error("Error: [toVALPrecondition] At least one condition should have been satisfied! Unrecognized StateFormula type.");
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
    if (c != 0) {
        VAL::expression* a1 = toVALExpression(&c->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&c->operand2(), nullptr);
        return new VAL::plus_expression(a1, a2);
    }

    const ppddl_parser::Subtraction* s = dynamic_cast<const ppddl_parser::Subtraction*>(exp);
    if (s != 0) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::minus_expression(a1, a2);
    }

    const ppddl_parser::Multiplication* m = dynamic_cast<const ppddl_parser::Multiplication*>(exp);
    if (m != 0) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::mul_expression(a1, a2);
    }

    const ppddl_parser::Division* d = dynamic_cast<const ppddl_parser::Division*>(exp);
    if (d != 0) {
        VAL::expression* a1 = toVALExpression(&s->operand1(), nullptr);
        VAL::expression* a2 = toVALExpression(&s->operand2(), nullptr);
        return new VAL::div_expression(a1, a2);
    }
    std::runtime_error("Error: [toVALExpression] At least one condition should have been satisfied! Unrecognized Expression type.");
}

VAL::effect_lists *VALConversion::toVALEffects(const ppddl_parser::Effect *e,
                                               const std::shared_ptr<ppddl_parser::Domain> &dom) {
    VAL::effect_lists* ef = new VAL::effect_lists();

    const ppddl_parser::SimpleEffect* se = dynamic_cast<const ppddl_parser::SimpleEffect*>(e);
    if (se != 0) {
        const ppddl_parser::Atom *a = &se->atom();

        VAL::pred_symbol *h = new VAL::pred_symbol(ppddl_parser::PredicateTable::name(a->predicate()));
        VAL::parameter_symbol_list *sl = new VAL::parameter_symbol_list;
        ppddl_parser::TypeList tl = ppddl_parser::PredicateTable::parameters(a->predicate());
        for (auto tlit = tl.begin(); tlit != tl.end(); ++tlit) {
            // get type name
            std::string t_name = dom->types().typestring(*tlit);

            sl->push_back(new VAL::var_symbol(t_name));
        }

        VAL::proposition *prop = new VAL::proposition(h, sl);

        const ppddl_parser::AddEffect* ae = dynamic_cast<const ppddl_parser::AddEffect*>(e);
        const ppddl_parser::DeleteEffect *de = dynamic_cast<const ppddl_parser::DeleteEffect *>(e);
        if (ae != 0) {
            ef->add_effects.push_back(new VAL::simple_effect(prop));
        }
        else if (de != 0) {
            ef->del_effects.push_back(new VAL::simple_effect(prop));
        }
    }

    /*pc_list<simple_effect*> add_effects;
pc_list<simple_effect*> del_effects;
pc_list<forall_effect*> forall_effects;
pc_list<cond_effect*>   cond_effects;
pc_list<cond_effect*>   cond_assign_effects;
pc_list<assignment*>    assign_effects;
pc_list<timed_effect*>  timed_effects;*/

    const ppddl_parser::UpdateEffect* ue = dynamic_cast<const ppddl_parser::UpdateEffect*>(e); // ie decrease / increase
    if (ue != 0) {
        //TODO ef->assign_effects.push_back();

    }

    const ppddl_parser::ConjunctiveEffect* ce = dynamic_cast<const ppddl_parser::ConjunctiveEffect*>(e);
    if (ce != 0) {
        ppddl_parser::EffectList cjts = ce->conjuncts();
        for (auto it = cjts.begin(); it != cjts.end(); ++it) {
            VAL::effect_lists* conjunct = toVALEffects(*it, dom);
            ef->append_effects(conjunct);
        }
    }

    const ppddl_parser::ConditionalEffect* cone = dynamic_cast<const ppddl_parser::ConditionalEffect*>(e);
    if (cone != 0) {

    }

    const ppddl_parser::QuantifiedEffect* qe = dynamic_cast<const ppddl_parser::QuantifiedEffect*>(e);
    if (qe != 0) {

    }

    const ppddl_parser::ProbabilisticEffect* pe = dynamic_cast<const ppddl_parser::ProbabilisticEffect*>(e);
    if (pe != 0) {
        std::runtime_error("Error: Probabilistic effects can not be converted to PDDL! Please determinize the domain first.");
    }
    std::runtime_error("Error: [toVALEffects] At least one condition should have been satisfied! Unrecognized Effect type.");
}

