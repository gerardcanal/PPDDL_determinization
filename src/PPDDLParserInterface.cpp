//
// Created by gcanal on 18/12/17.
//

#include "PPDDLParserInterface.h"
#include <cstring>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// DOMAIN //////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Name of current file. */
std::string current_file;
/* Level of warnings. */
int warning_level;

namespace PPDDLInterface {
    Domain::Domain(const std::string &domain_path, const std::vector<std::string> &problem_paths) {
        // Get the domain
        if (readPPDDLFile(domain_path)) {
            std::cout << "Domain file " << domain_path << " parsed correctly." << std::endl;
            int i = 0;
            for (p_Domain::DomainMap::const_iterator di = p_Domain::begin(); di != p_Domain::end(); di++, ++i) {
                if (i > 0) { // Ensures only one iteration
                    std::cerr << "More than one domain were found. Only the first one (" << _dom->name()
                              << ") will be used." << std::endl;
                    break;
                }
                _dom = std::shared_ptr<p_Domain>(const_cast<p_Domain *>(di->second));
                // Note: const_cast is used here as the domain is read here (and only here) and it avoids making a copy by
                //       using the other constructor, as well as avoids removing the const from the original file which I
                //       preferred to modify as less as possible.
            }
        } else {
            std::cerr << "There were errors while parsing input file!" << std::endl;
            throw(std::runtime_error("There was a Domain parsing error!"));
            //exit(-1);
        }

        for (auto ppath = problem_paths.begin(); ppath != problem_paths.end(); ++ppath) {
            if (readPPDDLFile(*ppath)) {
                std::cout << "Problem file " << *ppath << " parsed correctly." << std::endl;
            } else {
                std::cerr << "There were errors while parsing input file! Finishing the program." << std::endl;
                throw(std::runtime_error("There was a Problem parsing error!"));
            }
        }
    }

    Domain::~Domain() {}


    Domain::Domain(const Domain &d, const std::string &name_suffix) {
        const std::shared_ptr<p_Domain> p = d._dom;
        std::string new_name = p->name() + "_" + name_suffix;
        while (p_Domain::find(new_name) != nullptr) { // Domain already exists! Change its name...
            int id = 0;
            if (isdigit(new_name[new_name.size() - 1])) {
                id = std::atoi(&new_name[new_name.size() - 1]);
                new_name = new_name.substr(0, new_name.size() - 1);
            }
            new_name = p->name() + "_" + name_suffix + std::to_string(++id);
        }
        _dom = std::shared_ptr<p_Domain>(new p_Domain(new_name));

        /* Requirements */
        _dom->requirements = p->requirements;

        /* Domain types. */
        ppddl_parser::TypeTable types_ = p->types(); // A copy is made here
        _dom->types() = types_;

        /* Domain predicates. */
        ppddl_parser::PredicateTable predicates_ = p->predicates();
        _dom->predicates() = predicates_;

        /* Domain functions. */
        ppddl_parser::FunctionTable functions_ = p->functions();
        _dom->functions() = functions_;

        /* Domain terms. */
        ppddl_parser::TermTable terms_ = p->terms();
        _dom->terms() = terms_;

        /* Domain actions. */
        for (ppddl_parser::ActionSchemaMap::const_iterator ai = p->actions().begin(); ai != p->actions().end(); ai++) {
            p_actionSchema *action_ = new p_actionSchema(ai->first); // Allocate new actionSchema.
            action_->set_parameters(ai->second->parameters()); // Set parameters makes the copy
            action_->set_precondition(ai->second->precondition().clone()); // Where to add it?*

            const p_Effect *eff = &ai->second->effect().clone();
            action_->set_effect(*eff);
            RCObject::deref(eff);

            _dom->add_action(*action_);
        }
    }

    bool
    Domain::readPPDDLFile(const std::string &domain_path, int new_verbosity, int new_warning_level) {
        warning_level = new_warning_level;

        /* Parses the given file, and returns true on success. */
        ppddl_in = fopen(domain_path.c_str(), "r");
        if (ppddl_in == nullptr) {
            std::cerr << "PPDDLParserInterface: " << domain_path << ": " << strerror(errno)
                      << std::endl;
            return false;
        } else {
            current_file = domain_path;
            bool success = (ppddl_parse() == 0);
            fclose(ppddl_in);
            return success;
        }
    }

    Action Domain::getAction(const std::string &name) {
        const p_actionSchema *as = _dom->find_action(name);
        if (as == nullptr) throw std::runtime_error("ERROR: Unknown action name " + name + ".");
        return Action(as);
    }

    std::vector<Action> Domain::getActions() const {
        std::vector<Action> ret_actions(_dom->actions().size()); // Fixed size, no reallocations
        size_t i = 0;
        for (ppddl_parser::ActionSchemaMap::const_iterator ai = _dom->actions().begin();
             ai != _dom->actions().end(); ai++) {
            ret_actions[i++] = Action(ai->second);
        }
        return ret_actions;
    }

    void Domain::setAction(const Action &new_action) {
        _dom->add_action(
                *new_action._as); // As it is an ActionMap, it will override the action in case it is already there
        const_cast<Action*>(&new_action)->releasePtr(); // No harm as the public interface is not modified, it's just for internal memory management
    }

    std::shared_ptr<VALDomain> Domain::getVALDomain() {
        if (determinized()) {
            const p_Domain *p_domain = &*_dom;
            return VALConversion::toVALDomain(p_domain);
        } else throw std::runtime_error("ERROR! Non determinized domain can't be converted to (non-stochastic) PDDL!");
    }

    void Domain::printPDDL(const string &output_folder_path, string domain_name, string problem_name) {
        // Check pddl extension... Remove it for problem names, add it for domain names
        size_t pos = problem_name.find(".pddl");
        if (pos != string::npos) problem_name = problem_name.substr(0, pos);
        pos = domain_name.find(".pddl");
        if (domain_name.size() > 0 and pos == string::npos) domain_name += ".pddl";


        //VAL::domain val_d = *getVALDomain().get();
        std::shared_ptr<VALDomain> wrapper = getVALDomain();
        const std::shared_ptr<VAL::domain> val_d = wrapper->get();
        val_d->setWriteController(auto_ptr<VAL::WriteController>(new VAL::PDDLPrinter));

        if (domain_name.size() == 0) domain_name = val_d->name + "_domain.pddl";
        std::ofstream o(output_folder_path + "/" + domain_name);
        o << "; Do not edit! This file was generated automatically." << std::endl;
        o << *val_d;
        o.close();

        int i = 0;
        string pre_path_problem = output_folder_path + "/";
        for (auto prit = ppddl_parser::Problem::begin(); prit != ppddl_parser::Problem::end(); ++prit) {
            std::shared_ptr<VALProblem> p = VALConversion::toVALProblem(prit->second, wrapper);
            const std::shared_ptr<VAL::problem> val_p = p->get();
            val_p->setWriteController(auto_ptr<VAL::WriteController>(val_d->recoverWriteController()));

            string current_problem_name = pre_path_problem + problem_name + (i > 0? std::to_string(i): "")+ ".pddl";
            if (problem_name.size() == 0)
                current_problem_name = pre_path_problem + val_d->name + "_" + val_p->name + "_problem.pddl";
            o = std::ofstream(current_problem_name);

            o << "; Do not edit! This file was generated automatically." << std::endl;
            o << *val_p;
            o.close();
            ++i;
        }
    }

    bool Domain::determinized() {
        bool determ = true;
        for (ppddl_parser::ActionSchemaMap::const_iterator ai = _dom->actions().begin();
             ai != _dom->actions().end() && determ;
             ai++) {
            determ &= Effect::determinized(ai->second->effect());
        }
        return determ;
    }

    void Domain::printPPDDL(const string &output_folder_path, std::string domain_name) {
        if (domain_name.size() == 0) domain_name = _dom->name() + "_probabilistic_domain.pddl";
        std::ofstream o(output_folder_path + "/" + domain_name);
        o << "; Do not edit! This file was generated automatically." << std::endl;
        _dom->writePPDDL(o);
        o.close();

        for (auto prit = ppddl_parser::Problem::begin(); prit != ppddl_parser::Problem::end(); ++prit) {
            o = std::ofstream(output_folder_path + "/" + _dom->name() + "_probabilistic_" + prit->second->name() +
                              "_problem.pddl");
            o << "; Do not edit! This file was generated automatically." << std::endl;
            prit->second->writePPDDL(o, _dom->name());
            o.close();
        }
    }

    void Domain::loadProblems(const std::vector<std::string> &problem_paths) {
        for (auto it = problem_paths.begin(); it != problem_paths.end(); ++it) readPPDDLFile(*it);
    }

    void Domain::deleteAction(Action &action) {
        _dom->remove_action(action.getName());
    }

    std::string Domain::getMetric() {
        auto prit = ppddl_parser::Problem::begin();
        if (prit != ppddl_parser::Problem::end()) {
            std::string ret = "+";
            const ppddl_parser::Expression* metric = &prit->second->metric();
            const ppddl_parser::Subtraction* sub = dynamic_cast<const ppddl_parser::Subtraction*>(metric);
            if (sub != nullptr) {
                // The ppddl_parser always maximizes, and represents the minimization of X as maximize (- 0 X), so we check if it's a 0-X case
                const ppddl_parser::Value* op1 = dynamic_cast<const ppddl_parser::Value*>(&sub->operand1());
                if (op1 != nullptr && op1->value().double_value()==0) {
                    // It is a 0-X case, so the metric is minimizing, and the value is the operand2.
                    ret = "-"; //op = VAL::optimization::E_MINIMIZE;
                    metric = &sub->operand2();
                }

            }
            const ppddl_parser::Fluent* f = dynamic_cast<const ppddl_parser::Fluent*>(metric);
            if (f == nullptr) throw std::runtime_error("Error: Expecting metric of type (:metric minimize/maximize (<cost_function_name>)");
            return ret + ppddl_parser::FunctionTable::name(f->function());
        }
        return "";
    }

    std::string Domain::getName() {
        return _dom->name();
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Actions /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Action::Action(const p_actionSchema *as, const std::string& name_suffix) {
        if (as != nullptr) {
            initFrom(as, name_suffix);
            _delete_actionschema = true;
        }
        else {
            _delete_actionschema = false;
            _as = nullptr;
        }
    }


    Action::Action() { _delete_actionschema = true; }

    Action::Action(const Action &a, const std::string& name_suffix) : Action(a._as, name_suffix) {
    }


    Action::~Action() {
        if (_delete_actionschema) delete _as;
    }

    std::shared_ptr<Effect> Action::getEffect() const {
        return _action_effect; // By polymorfism it'll return the correct type
    }

    void Action::setEffect(const Effect &e) {
        const p_Effect *effect_ptr = &e.getEffect()->clone();
        _as->set_effect(*effect_ptr);

        setRawEffectPtr(effect_ptr);
        RCObject::deref(effect_ptr); // As effect_ptr will be removed here, we have to decrement the counter

        _action_effect->releasePtr(); // As it will remain in _as, the pointer doesn't have to be deleted by the interface
    }

    std::string Action::getName() const {
        return _as->name();
    }

    void Action::setRawEffectPtr(const p_Effect *e) {
        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(e);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.
            _action_effect = std::shared_ptr<Effect>(new ConjunctiveEffect(ce));
        } else {
            const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(e);
            if (pe != nullptr) {
                _action_effect = std::shared_ptr<Effect>(new ProbabilisticEffect(pe));
            } else {
                // it is another effect
                _action_effect = std::shared_ptr<Effect>(new Effect(e));
            }
        }
    }

    void Action::releasePtr() {
        _delete_actionschema = false;
    }

    Action &Action::operator=(const Action &other) {
        initFrom(other._as);
        return *this;
    }

    void Action::initFrom(const p_actionSchema *as, const std::string& name_suffix) {
        _as = new p_actionSchema(as->name() + name_suffix);
        _as->set_parameters(as->parameters()); // Set parameters makes the copy
        _as->set_precondition(as->precondition().clone());
        _as->set_effect(as->effect().clone());

        const p_Effect *e = &_as->effect();
        RCObject::deref(e); // As the set effect increments the reference, and the clone() does too, we have to
        // decrement once the reference here -> e pointer will disappear when it's out of scope, so it
        // will have one less reference

        setRawEffectPtr(e);
    }

    double Action::getCost(const string &metric) {
        return _action_effect->getCost(metric);
    }

    void Action::setCost(double cost, const string &metric) {
        return _action_effect->setCost(cost, metric);
    }

    p_Update *Action::getCostFunction(const std::string &metric) {
        return _action_effect->getCostFunction(metric);
    }

    void Action::setCostFunction(const p_Update *up, const string &metric) {
        _action_effect->setCostFunction(up, metric);
    }

    ActionList::ActionList() : Action(nullptr){
        _delete_actionschema = false;
    }

    ActionList::ActionList(size_t n) : ActionList() {
        _actions.reserve(n);
    }

    void ActionList::addAction(const Action& a) {
        _actions.push_back(std::make_shared<PPDDLInterface::Action>(a));
    }

    size_t ActionList::size() const {
        return _actions.size();
    }

    ActionList::ActionList(const ActionList &al) : ActionList() {
        _actions = al._actions;
    }


    ActionPtr makePtr(const Action& e) {
        return std::make_shared<Action>(e);
    }

    ActionPtr makePtr(const ActionList& e) {
        return std::make_shared<ActionList>(e);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Effects ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ProbabilisticEffect::ProbabilisticEffect(const p_ProbabilisticEffect *e) : Effect(e) {
    }

    size_t ProbabilisticEffect::size() const {
        return constEffect()->size();
    }

    double ProbabilisticEffect::getProbability(size_t i) const {
        return constEffect()->probability(i).double_value();
    }

    void ProbabilisticEffect::setProbability(double p, size_t i) {
        ppddl_parser::Rational prat(p*100000,100000);
        modificableEffect()->probability(prat, i);
    }

    Effect ProbabilisticEffect::getEffect(size_t i) const {
        Effect ret(&constEffect()->effect(i).clone());
        RCObject::deref(ret.getEffect()); // As the clone increments the reference and the constructor does too
        return ret;
    }

    const p_ProbabilisticEffect *ProbabilisticEffect::constEffect() const {
        return static_cast<const p_ProbabilisticEffect *>(_eff);
    }

    p_ProbabilisticEffect *ProbabilisticEffect::modificableEffect() const {
        return const_cast<p_ProbabilisticEffect *>(constEffect());
        // We remove const to be able to successfully modify the effect
    }

    ConjunctiveEffect::ConjunctiveEffect(const p_ConjunctiveEffect *e) : Effect(e) {
    }


    ConjunctiveEffect::ConjunctiveEffect(const ConjunctiveEffect &e)
            : Effect(&e._eff->clone()) {
        RCObject::deref(_eff);
    }

    size_t ConjunctiveEffect::size() const {
        return constEffect()->conjuncts().size();
    }

    EffectPtr ConjunctiveEffect::getConjunct(size_t i) const {
        const p_Effect *cjt = &constEffect()->conjuncts()[i]->clone();
        RCObject::deref(cjt); // Decrement the reference as the constructor for Effect will increase it

        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(cjt);
        if (pe != nullptr) {
            return std::shared_ptr<Effect>(new ProbabilisticEffect(pe));
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(cjt);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.
            return std::shared_ptr<Effect>(new ConjunctiveEffect(ce));
        }

        return std::shared_ptr<Effect>(new Effect(cjt));
    }

    void ConjunctiveEffect::changeConjunct(const Effect &cjct, size_t i) {
        p_EffectList cj_copy = constEffect()->conjuncts();
        RCObject::destructive_deref(cj_copy[i]); // Dereference and delete it as we're overwriting it
        cj_copy[i] = cjct.getEffect();
        RCObject::ref(cj_copy[i]); // Increase the reference as we made a copy
        modificableEffect()->set_conjuncts(cj_copy);
        // FIXME This makes unnecessary copies... Maybe could be optimised?
    }

    const p_ConjunctiveEffect *ConjunctiveEffect::constEffect() const {
        return static_cast<const p_ConjunctiveEffect *>(getEffect());
        // If it is inside the Conjunctive Effect class we are sure that the cast is okay
    }

    p_ConjunctiveEffect *ConjunctiveEffect::modificableEffect() const {
        return const_cast<p_ConjunctiveEffect *>(constEffect());
        // We remove const to be able to successfully modify the effect
    }

    Effect::Effect(const p_Effect *e) {
        _eff = e;
        if (e != nullptr) {
            RCObject::ref(_eff);
            _delete_ptr = true;
        }
        else _delete_ptr = false;
    }


    Effect::Effect(const Effect &e) {
        if (e._eff != nullptr) {
            this->_eff = &e._eff->clone();
            this->_delete_ptr = e._delete_ptr;
        }
    }

    Effect &Effect::operator=(const Effect &other) {
        RCObject::destructive_deref(this->_eff); // As the _eff will be overwritten, we need to decrement one reference!
        this->_eff = &other._eff->clone();
        this->_delete_ptr = other._delete_ptr;
        return *this;
    }

    const p_Effect *Effect::getEffect() const {
        return _eff;
    }

    Effect::~Effect() {
        if (_delete_ptr) RCObject::destructive_deref(_eff);//
    }

    void Effect::releasePtr() {
        _delete_ptr = false;
        RCObject::deref(
                _eff); // As we are releasing the pointer, it's like we're losing one reference as there won't be a delete in this case

    }

    bool Effect::determinized(const p_Effect &effect) {
        bool result = true;

        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(&effect);
        if (pe != nullptr) {
            return false;
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(&effect);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

            for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
                result &= Effect::determinized(**it);
            }
        }

        return result;
    }


    double Effect::getCost(const string &metric) {
        return getCost(*_eff, metric);
    }

    void Effect::setCost(double cost, const string &metric) {
        setCost(*_eff, cost, metric);
    }

    p_Update* Effect::getCostFunction(const string &metric) {
        return getCostFunction(*_eff, metric);
    }

    void Effect::setCostFunction(const p_Update* up, const string &metric) {
        setCostFunction(*_eff, up, metric);
    }

    double Effect::getCost(const p_Effect &effect, const string &metric) {
        double cost = 0;
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect*>(&effect);
        if (pe != nullptr) {
            for (size_t i = 0; i != pe->size(); ++i) {
                cost += getCost(pe->effect(i), metric);
            }
            return cost;
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(&effect);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

            for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
                cost += getCost(**it, metric);
            }
            return cost;
        }

        const ppddl_parser::UpdateEffect *ue = dynamic_cast<const ppddl_parser::UpdateEffect *>(&effect);
        if (ue != nullptr) {
            std::string metric_name = ppddl_parser::FunctionTable::name(ue->update().fluent().function());
            if (metric == metric_name) {
                const ppddl_parser::Value *val = dynamic_cast<const ppddl_parser::Value *>(&ue->update().expression());
                if (val == nullptr)
                    throw std::runtime_error("Error: Only simple value increase/decrease can be accessed.");
                double d_val = val->value().double_value();
                return d_val;
            }
        }
        return 0;
    }

    p_Update * Effect::getCostFunction(const p_Effect &effect, const std::string &metric) {
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect*>(&effect);
        if (pe != nullptr) { // TODO return list of effects?
            for (size_t i = 0; i != pe->size(); ++i) {
                p_Update * ue = getCostFunction(pe->effect(i), metric);
                if (ue != nullptr) return ue;
            }
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(&effect);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

            for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
                p_Update * ue = getCostFunction(**it, metric);
                if (ue != nullptr) return ue;
            }
        }

        const ppddl_parser::UpdateEffect *ue = dynamic_cast<const ppddl_parser::UpdateEffect *>(&effect);
        if (ue != nullptr) {
            std::string metric_name = ppddl_parser::FunctionTable::name(ue->update().fluent().function());
            if (metric == metric_name) {
                return const_cast<ppddl_parser::Update*>(&ue->update());
            }
        }
        return nullptr;
    }


    void Effect::setCost(const p_Effect &effect, double cost, const string &metric) {
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect*>(&effect);
        if (pe != nullptr) {
            for (size_t i = 0; i != pe->size(); ++i) {
                setCost(pe->effect(i), cost, metric);
            }
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(&effect);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

            for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
                setCost(**it, cost, metric);
            }
        }

        const ppddl_parser::UpdateEffect *ue = dynamic_cast<const ppddl_parser::UpdateEffect *>(&effect);
        if (ue != nullptr) {
            std::string metric_name = ppddl_parser::FunctionTable::name(ue->update().fluent().function());
            if (metric == metric_name) {
                // Rational value
                int multiplier = 1000000;
                ppddl_parser::Value* new_cost_val = new ppddl_parser::Value(ppddl_parser::Rational(cost * multiplier, multiplier));

                ppddl_parser::Update* new_update; // Updated update
                const ppddl_parser::Increase* inc = dynamic_cast<const ppddl_parser::Increase*>(&ue->update());
                const ppddl_parser::Decrease* dec = dynamic_cast<const ppddl_parser::Decrease*>(&ue->update());
                if (inc != nullptr) {
                    new_update = new ppddl_parser::Increase(ue->update().fluent(), *new_cost_val);
                    RCObject::destructive_deref(&ue->update().expression());
                }
                else if (dec != nullptr) {
                    new_update = new ppddl_parser::Decrease(ue->update().fluent(), *new_cost_val);
                    RCObject::destructive_deref(&ue->update().expression());
                }
                else throw std::runtime_error("Error: Only updates of type increase/decrease value can be modified.");

                // Modify the update. To do so, I get the editable pointer to the update and override it.
                // references are updated accordingly, deleting those that will not be used anymore and updating the new
                // ones.
                ppddl_parser::Update* edit_upd = const_cast<ppddl_parser::Update*>(&ue->update());
                RCObject::destructive_deref(&edit_upd->fluent());
                //RCObject::destructive_deref(&edit_upd->expression()); // This one is not needed.
                *edit_upd = *new_update; // (Safe) Const cast again to be able to modify it...
                RCObject::ref(&new_update->expression()); // As the delete will destroy one reference
                RCObject::ref(&new_update->fluent()); // As the delete will destroy one reference
                delete new_update; // As its value has been copied, free the pointer
            }
        }
    }


    void Effect::setCostFunction(const p_Effect &effect, const p_Update* costfunc, const string &metric) {
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect*>(&effect);
        if (pe != nullptr) {
            for (size_t i = 0; i != pe->size(); ++i) {
                setCostFunction(pe->effect(i), costfunc, metric);
            }
        }

        const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect *>(&effect);
        if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

            for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
                setCostFunction(**it, costfunc, metric);
            }
        }

        const ppddl_parser::UpdateEffect *ue = dynamic_cast<const ppddl_parser::UpdateEffect *>(&effect);
        if (ue != nullptr) {
            std::string metric_name = ppddl_parser::FunctionTable::name(ue->update().fluent().function());
            if (metric == metric_name) {
                // Modify the update. To do so, I get the editable pointer to the update and override it.
                // references are updated accordingly, deleting those that will not be used anymore and updating the new
                // ones.
                ppddl_parser::Update* edit_upd = const_cast<ppddl_parser::Update*>(&ue->update());
                RCObject::destructive_deref(&edit_upd->fluent());
                //RCObject::destructive_deref(&edit_upd->expression()); // This one is not needed. FIXME check if needed
                *edit_upd = *costfunc; // (Safe) Const cast again to be able to modify it...
                //RCObject::ref(&edit_upd->expression()); // Not needed as costfunction is lost here
                RCObject::ref(&edit_upd->fluent()); // As the delete will destroy one reference
            }
        }
    }

    bool Effect::probabilitic() {
        return dynamic_cast<const p_ProbabilisticEffect*>(_eff) != nullptr;
    }

    bool Effect::conjunctive() {
        return dynamic_cast<const p_ConjunctiveEffect*>(_eff) != nullptr;
    }

    EffectList::EffectList() : Effect(nullptr) {
    }

    EffectList::EffectList(const ProbabilisticEffect &pe) : Effect(nullptr) {
        for (size_t o = 0; o < pe.size(); ++o) {
            addEffect(pe.getEffect(o), pe.getProbability(o));
        }
    }

    void EffectList::addEffect(const Effect &e, double w) {
        _effects.push_back(std::shared_ptr<Effect>(new Effect(e)));
        _weights.push_back(w);
    }


    EffectList::EffectList(size_t n) : Effect(nullptr) {
        _effects.reserve(n);
        _weights.reserve(n);
    }

    size_t EffectList::size() const {
        return _effects.size();
    }

    EffectList::EffectList(const EffectList &ef) : Effect(nullptr) {
        _effects = ef._effects;
        _weights = ef._weights;
    }

    EffectPtr makePtr(const Effect &e) {
        return std::make_shared<Effect>(e);
    }

    EffectPtr makePtr(const ConjunctiveEffect &e) {
        return std::make_shared<ConjunctiveEffect>(e);
    }

    EffectPtr makePtr(const ProbabilisticEffect &e) {
        return std::make_shared<ProbabilisticEffect>(e);
    }

    EffectPtr makePtr(const EffectList &e) {
        return std::make_shared<EffectList>(e);
    }
}