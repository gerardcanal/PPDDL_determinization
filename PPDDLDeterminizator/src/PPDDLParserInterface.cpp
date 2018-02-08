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

PPDDLInterface::Domain::Domain(const std::string &path) {
    if (readDomain(path)) {
        std::cout << "Domain file " << path << " parsed correctly." << std::endl;
        int i = 0;
        for (p_Domain::DomainMap::const_iterator di = p_Domain::begin(); di != p_Domain::end(); di++, ++i) {
            if (i > 0) { // Ensures only one iteration
                std::cerr << "More than one domain were found. Only the first one (" << _dom->name() << ") will be used." << std::endl;
                break;
            }
            _dom = std::shared_ptr<p_Domain>(const_cast<p_Domain*>(di->second));
            // Note: const_cast is used here as the domain is read here (and only here) and it avoids making a copy by
            //       using the other constructor, as well as avoids removing the const from the original file which I
            //       preferred to modify as less as possible.
        }
    }
    else  {
        std::cerr << "There were errors while parsing input file! Finishing the program." << std::endl;
        exit(-1);
    }
}

PPDDLInterface::Domain::~Domain() {
}


PPDDLInterface::Domain::Domain(const PPDDLInterface::Domain &d, const std::string &name_suffix) {
    const std::shared_ptr<p_Domain> p = d._dom;
    _dom = std::shared_ptr<p_Domain>(new p_Domain(p->name()+ "_" + name_suffix));

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
        p_actionSchema* action_ = new p_actionSchema(ai->first); // Allocate new actionSchema.
        action_->set_parameters(ai->second->parameters()); // Set parameters makes the copy
        action_->set_precondition(ai->second->precondition().clone()); // Where to add it?*

        const p_Effect* eff = &ai->second->effect().clone();
        action_->set_effect(*eff);
        RCObject::deref(eff);

        _dom->add_action(*action_);
    }
}

bool PPDDLInterface::Domain::readDomain(const std::string &domain_path, int new_verbosity, int new_warning_level) {
    warning_level = new_warning_level;

    /* Parses the given file, and returns true on success. */
    ppddl_in = fopen(domain_path.c_str(), "r");
    if (ppddl_in == nullptr) {
        std::cerr << "mdpclient:" << domain_path << ": " << strerror(errno)
                  << std::endl;
        return false;
    } else {
        current_file = domain_path;
        bool success = (ppddl_parse() == 0);
        fclose(ppddl_in);
        return success;
    }
}

PPDDLInterface::Action PPDDLInterface::Domain::getAction(const std::string &name) {
    const p_actionSchema* as = _dom->find_action(name);
    if (as == nullptr) throw std::runtime_error("ERROR: Unknown action name " + name + ".");
    return PPDDLInterface::Action(as);
}

std::vector<PPDDLInterface::Action> PPDDLInterface::Domain::getActions() const {
    std::vector<PPDDLInterface::Action> ret_actions(_dom->actions().size()); // Fixed size, no reallocations
    size_t i = 0;
    for (ppddl_parser::ActionSchemaMap::const_iterator ai = _dom->actions().begin(); ai != _dom->actions().end(); ai++) {
       ret_actions[i++] = PPDDLInterface::Action(ai->second);
    }
    return ret_actions;
}

void PPDDLInterface::Domain::setAction(const PPDDLInterface::Action& new_action) {
    _dom->add_action(*new_action._as); // As it is an ActionMap, it will override the action in case it is already there
    const_cast<PPDDLInterface::Action*>(&new_action)->releasePtr();
}

std::shared_ptr<VALDomain> PPDDLInterface::Domain::getVALDomain() {
    if (determinized()) {// TODO
        const p_Domain *p_domain = &*_dom;
        return VALConversion::toVALDomain(p_domain);
    }
    else throw std::runtime_error("ERROR! Non determinized domain can't be converted to (non-stochastic) PDDL!");
}

void PPDDLInterface::Domain::printPDDL(const string &output_folder_path) {
    //VAL::domain val_d = *getVALDomain().get();
    std::shared_ptr<VALDomain> wrapper = getVALDomain();
    const std::shared_ptr<VAL::domain> val_d = wrapper->get();
    val_d->setWriteController(auto_ptr<VAL::WriteController>(new VAL::PDDLPrinter));

    std::ofstream o(output_folder_path + "/" + val_d->name + "_domain.pddl");
    o << *val_d;
    o.close();

    for (auto prit = ppddl_parser::Problem::begin(); prit != ppddl_parser::Problem::end(); ++prit) {
        std::shared_ptr<VALProblem> p = VALConversion::toVALProblem(prit->second, wrapper);
        const std::shared_ptr<VAL::problem> val_p = p->get();
        val_p->setWriteController(auto_ptr<VAL::WriteController>(val_d->recoverWriteController()));
        o = std::ofstream(output_folder_path + "/" + val_d->name + "_" + val_p->name + "_problem.pddl");
        o << *val_p;
        o.close();
    }
}

bool PPDDLInterface::Domain::determinized() {
    bool determ = true;
    for (ppddl_parser::ActionSchemaMap::const_iterator ai = _dom->actions().begin();
         ai != _dom->actions().end() && determ;
         ai++) {
        determ &= Effect::determinized(ai->second->effect());
    }
    return determ;
}

void PPDDLInterface::Domain::printPPDDL(const string &output_folder_path) {
    std::ofstream o(output_folder_path + "/" + _dom->name() + "_probabilistic_domain.pddl");
    _dom->writePPDDL(o);
    o.close();

    for (auto prit = ppddl_parser::Problem::begin(); prit != ppddl_parser::Problem::end(); ++prit) {
        o = std::ofstream(output_folder_path + "/" + _dom->name() + "_probabilistic_" + prit->second->name() + "_problem.pddl.pddl");
        prit->second->writePPDDL(o, _dom->name());
        o.close();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Actions /////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PPDDLInterface::Action::Action(const p_actionSchema* as) {
    initFrom(as);
    _delete_actionschema = true;
}


PPDDLInterface::Action::Action() {_delete_actionschema = true;}

PPDDLInterface::Action::Action(const PPDDLInterface::Action &a) : Action(a._as){
}


PPDDLInterface::Action::~Action() {
    if (_delete_actionschema) delete _as;
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::Action::getEffect() const {
    return _action_effect; // By polymorfism it'll return the correct type
}

void PPDDLInterface::Action::setEffect(const PPDDLInterface::Effect &e) {
    const p_Effect* effect_ptr = &e.getEffect()->clone();
    _as->set_effect(*effect_ptr);

    setRawEffectPtr(effect_ptr);
    RCObject::deref(effect_ptr); // As effect_ptr will be removed here, we have to decrement the counter

    _action_effect->releasePtr(); // As it will remain in _as, the pointer doesn't have to be deleted by the interface
}

std::string PPDDLInterface::Action::getName() const {
    return _as->name();
}

void PPDDLInterface::Action::setRawEffectPtr(const PPDDLInterface::p_Effect *e) {
    const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect*>(e);
    if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.
        _action_effect = std::shared_ptr<PPDDLInterface::Effect>(new PPDDLInterface::ConjunctiveEffect(ce));
    }
    else {
        const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(e);
        if (pe != nullptr) {
            _action_effect = std::shared_ptr<PPDDLInterface::Effect>(new PPDDLInterface::ProbabilisticEffect(pe));
        }
        else {
            // it is another effect
            _action_effect = std::shared_ptr<PPDDLInterface::Effect>(new PPDDLInterface::Effect(e));
        }
    }
}

void PPDDLInterface::Action::releasePtr() {
    _delete_actionschema = false;
}

PPDDLInterface::Action &PPDDLInterface::Action::operator=(const PPDDLInterface::Action &other) {
    initFrom(other._as);
    return *this;
}

void PPDDLInterface::Action::initFrom(const PPDDLInterface::p_actionSchema *as) {
    _as = new p_actionSchema(as->name());
    _as->set_parameters(as->parameters()); // Set parameters makes the copy
    _as->set_precondition(as->precondition().clone());
    _as->set_effect(as->effect().clone());

    const p_Effect* e = &_as->effect();
    RCObject::deref(e); // As the set effect increments the reference, and the clone() does too, we have to
                        // decrement once the reference here -> e pointer will disappear when it's out of scope, so it
                        // will have one less reference

    setRawEffectPtr(e);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Effects ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PPDDLInterface::ProbabilisticEffect::ProbabilisticEffect(const PPDDLInterface::p_ProbabilisticEffect *e) : Effect(e) {
}

size_t PPDDLInterface::ProbabilisticEffect::size() const {
    return constEffect()->size();
}

double PPDDLInterface::ProbabilisticEffect::getProbability(size_t i) const {
    return constEffect()->probability(i).double_value();
}

PPDDLInterface::Effect PPDDLInterface::ProbabilisticEffect::getEffect(size_t i) const {
    PPDDLInterface::Effect ret(&constEffect()->effect(i).clone());
    RCObject::deref(ret.getEffect()); // As the clone increments the reference and the constructor does too
    return ret;
}

const PPDDLInterface::p_ProbabilisticEffect *PPDDLInterface::ProbabilisticEffect::constEffect() const {
    return static_cast<const p_ProbabilisticEffect*>(_eff);
}

PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::p_ConjunctiveEffect *e): Effect(e) {
}


PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::ConjunctiveEffect &e) : PPDDLInterface::Effect(&e._eff->clone()) {
    RCObject::deref(_eff);
}

size_t PPDDLInterface::ConjunctiveEffect::size() const {
    return constEffect()->conjuncts().size();
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::ConjunctiveEffect::getConjunct(size_t i) const {
    const p_Effect* cjt = &constEffect()->conjuncts()[i]->clone();
    RCObject::deref(cjt); // Decrement the reference as the constructor for PPDDLInterface::Effect will increase it

    const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(cjt);
    if (pe != nullptr) {
        return std::shared_ptr<PPDDLInterface::Effect>(new ProbabilisticEffect(pe));
    }

    const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect*>(cjt);
    if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.
        return std::shared_ptr<PPDDLInterface::Effect>(new ConjunctiveEffect(ce));
    }

    return std::shared_ptr<PPDDLInterface::Effect>(new Effect(cjt));
}

void PPDDLInterface::ConjunctiveEffect::changeConjunct(const PPDDLInterface::Effect &cjct, size_t i) {
    EffectList cj_copy = constEffect()->conjuncts();
    RCObject::destructive_deref(cj_copy[i]); // Dereference and delete it as we're overwriting it
    cj_copy[i] = cjct.getEffect();
    RCObject::ref(cj_copy[i]); // Increase the reference as we made a copy
    modificableEffect()->set_conjuncts(cj_copy);
    // FIXME This makes unnecessary copies... Maybe could be optimised?
}

const PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::constEffect() const {
    return  static_cast<const p_ConjunctiveEffect*>(getEffect());
    // If it is inside the Conjunctive Effect class we are sure that the cast is okay
}

PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::modificableEffect() const {
    return const_cast<p_ConjunctiveEffect*>(constEffect());
    // We remove const to be able to successfully modify the effect
}

PPDDLInterface::Effect::Effect(const PPDDLInterface::p_Effect *e) {
    _eff = e;
    RCObject::ref(_eff);
    _delete_ptr = true;
}


PPDDLInterface::Effect::Effect(const Effect &e) {
    this->_eff = &e._eff->clone();
    this->_delete_ptr = e._delete_ptr;
}

PPDDLInterface::Effect &PPDDLInterface::Effect::operator=(const Effect &other) {
    RCObject::destructive_deref(this->_eff); // As the _eff will be overwritten, we need to decrement one reference!
    this->_eff = &other._eff->clone();
    this->_delete_ptr = other._delete_ptr;
    return *this;
}

const PPDDLInterface::p_Effect* PPDDLInterface::Effect::getEffect() const {
    return _eff;
}

PPDDLInterface::Effect::~Effect() {
    if (_delete_ptr) RCObject::destructive_deref(_eff);//
}

void PPDDLInterface::Effect::releasePtr() {
    _delete_ptr = false;
    RCObject::deref(_eff); // As we are releasing the pointer, it's like we're losing one reference as there won't be a delete in this case

}

bool PPDDLInterface::Effect::determinized(const p_Effect &effect) {
    bool result = true;

    const p_ProbabilisticEffect *pe = dynamic_cast<const p_ProbabilisticEffect *>(&effect);
    if (pe != nullptr) {
        return false;
    }

    const p_ConjunctiveEffect *ce = dynamic_cast<const p_ConjunctiveEffect*>(&effect);
    if (ce != nullptr) { // Anidate ifs to avoid unneeded dynamic casts.

        for (auto it = ce->conjuncts().begin(); it != ce->conjuncts().end(); ++it) {
            result &= Effect::determinized(**it);
        }
    }

    return result;
}