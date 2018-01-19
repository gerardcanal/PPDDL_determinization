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
    else std::cerr << "There were errors while parsing input file!" << std::endl;
}

PPDDLInterface::Domain::~Domain() {
    //delete _dom;
}


PPDDLInterface::Domain::Domain(const PPDDLInterface::Domain& d) {
    const std::shared_ptr<p_Domain> p = d._dom;
    _dom = std::shared_ptr<p_Domain>(new p_Domain(p->name()+ "_det"));

    /* Domain types. */
    TypeTable types_ = p->types(); // A copy is made here
    _dom->types() = types_;

    /* Domain predicates. */
    PredicateTable predicates_ = p->predicates();
    _dom->predicates() = predicates_;

    /* Domain functions. */
    FunctionTable functions_ = p->functions();
    _dom->functions() = functions_;

    /* Total-time and goal-achieved are created in the Domains creator
    // The `total-time' function. //
    Function total_time_ = p->total_time();
    _dom->total_time() = total_time_;

    // The `goal-achieved' function. //
    Function goal_achieved_ = p->goal_achieved();
    _dom->goal_achieved() = goal_achieved_;*/

    /* Domain terms. */
    TermTable terms_ = p->terms();
    _dom->terms() = terms_;

    /* Domain actions. */
    //ActionSchemaMap actions_; // Copy all the actions!!
    for (ActionSchemaMap::const_iterator ai = p->actions().begin(); ai != p->actions().end(); ai++) {
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
    yyin = fopen(domain_path.c_str(), "r");
    if (yyin == nullptr) {
        std::cerr << "mdpclient:" << domain_path << ": " << strerror(errno)
                  << std::endl;
        return false;
    } else {
        current_file = domain_path;
        bool success = (ppddl_parse() == 0);
        fclose(yyin);
        return success;
    }
}

PPDDLInterface::Action PPDDLInterface::Domain::getAction(const std::string &name) {
    const p_actionSchema* as = _dom->find_action(name);
    if (as == nullptr) std::runtime_error("ERROR: Unknown action name " + name + ".");
    return PPDDLInterface::Action(as);
}

std::vector<PPDDLInterface::Action> PPDDLInterface::Domain::getActions() const {
    std::vector<PPDDLInterface::Action> ret_actions(_dom->actions().size()); // Fixed size, no reallocations
    size_t i = 0;
    for (ActionSchemaMap::const_iterator ai = _dom->actions().begin(); ai != _dom->actions().end(); ai++) {
       ret_actions[i++] = PPDDLInterface::Action(ai->second);
    }
    return ret_actions;
}

void PPDDLInterface::Domain::setAction(const PPDDLInterface::Action& new_action) {
    _dom->add_action(*new_action._as); // As it is an ActionMap, it will override the action in case it is already there
    const_cast<PPDDLInterface::Action*>(&new_action)->releasePtr();
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
    // The &(*a._as) is done to get the pointer and ease the interface to the class. it's safe as the consctructor
    //     also makes a copy.
    /* FIXME remove
    p_actionSchema *as = new p_actionSchema(a.getName());
    as->set_parameters(a._as->parameters()); // Set parameters makes the copy
    as->set_precondition(a._as->precondition().clone());
    as->set_effect(a._as->effect().clone());
    _as = as;*/
}


PPDDLInterface::Action::~Action() {
    if (_delete_actionschema) delete _as;
    //RCObject::destructive_deref(_action_effect->getEffect());
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::Action::getEffect() const {
    return _action_effect; // By polymorfism it'll return the correct type
}

void PPDDLInterface::Action::setEffect(const PPDDLInterface::Effect &e) {
    const p_Effect* effect_ptr = &e.getEffect()->clone();
    _as->set_effect(*effect_ptr);

    setRawEffectPtr(effect_ptr);
    RCObject::deref(effect_ptr); // As effect_ptr will be removed here, we have to decrement the counter

    _action_effect->releasePtr(); // As it will remain in _as, the pointer doesn't have to be deleted by _as
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

PPDDLInterface::ProbabilisticEffect::ProbabilisticEffect(const PPDDLInterface::p_ProbabilisticEffect *e) : Effect(e) {// FIXME should be clone?!
    //_pe = const_cast<p_ProbabilisticEffect*>(dynamic_cast<const p_ProbabilisticEffect*>(&e->clone()));
    // We copy it and remove const to be able to successfully modify the effect
}

size_t PPDDLInterface::ProbabilisticEffect::size() const {
    return constEffect()->size();
}

double PPDDLInterface::ProbabilisticEffect::getProbability(size_t i) const {
    return constEffect()->probability(i).double_value();
}

PPDDLInterface::Effect PPDDLInterface::ProbabilisticEffect::getEffect(size_t i) const {
    PPDDLInterface::Effect ret(&constEffect()->effect(i).clone()); // FIXME ensure each clone's deletion!!!!
    RCObject::deref(ret.getEffect()); // As the clone increments the reference and the constructor does too
    return ret;
}

const PPDDLInterface::p_ProbabilisticEffect *PPDDLInterface::ProbabilisticEffect::constEffect() const {
    return static_cast<const p_ProbabilisticEffect*>(_eff);
}

PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::p_ConjunctiveEffect *e): Effect(e) {
    //_ce = const_cast<p_ConjunctiveEffect*>(dynamic_cast<const p_ConjunctiveEffect*>(&e->clone()));
    // We copy it and remove const to be able to successfully modify the effect
}


PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::ConjunctiveEffect &e) : PPDDLInterface::Effect(&e._eff->clone()) {// FIXME ensure each clone's deletion!!!!
    RCObject::deref(_eff);
}

size_t PPDDLInterface::ConjunctiveEffect::size() const {
    return constEffect()->conjuncts().size();
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::ConjunctiveEffect::getConjunct(size_t i) const {
    const p_Effect* cjt = &constEffect()->conjuncts()[i]->clone(); // FIXME ensure each clone's deletion!!!!
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
    // This makes unnecessary copies... Maybe could be optimised?
}

const PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::constEffect() const {
    return  static_cast<const p_ConjunctiveEffect*>(getEffect());
    // If it is inside the Conjunctive Effect class we are sure that the cast is okay
}

PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::modificableEffect() const {
    return const_cast<p_ConjunctiveEffect*>(constEffect());
    // We remove const to be able to successfully modify the effect;
}

PPDDLInterface::Effect::Effect(const PPDDLInterface::p_Effect *e) {
    _eff = e;
    RCObject::ref(_eff);
    _delete_ptr = true;
}


PPDDLInterface::Effect::Effect(const Effect &e) {
    //this->_eff = &e._eff->clone();
    this->_eff = e._eff; // FIXME should clone? :/
    this->_delete_ptr = e._delete_ptr;
    RCObject::ref(_eff);
}

PPDDLInterface::Effect &PPDDLInterface::Effect::operator=(const Effect &other) {
    //this->_eff = &other._eff->clone();
    this->_eff = other._eff; // FIXME should clone? :/
    this->_delete_ptr = other._delete_ptr;
    RCObject::ref(_eff);
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