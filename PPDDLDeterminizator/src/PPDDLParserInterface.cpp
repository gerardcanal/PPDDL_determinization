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
        p_actionSchema* action_ = new p_actionSchema(ai->first); // Allocate new actionSchema. TODO It MUST be deleted in the destructor
        action_->set_parameters(ai->second->parameters()); // Set parameters makes the copy
        action_->set_precondition(ai->second->precondition().clone()); // Where to add it?*
        action_->set_effect(ai->second->effect().clone());
        _dom->add_action(*action_); // FIXME ensure deletion of above news inside p_actionSchema
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
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::Action::getEffect() const {
    return _action_effect; // By polymorfism it'll return the correct type
}

void PPDDLInterface::Action::setEffect(const PPDDLInterface::Effect &e) {
    /**_action_effect = e;
    _as->set_effect(e.getEffect()->clone()); // FIXME check clone is released*/
    const p_Effect* effect_ptr = &e.getEffect()->clone();
    setRawEffectPtr(effect_ptr); // FIXME check clone is released
    _as->set_effect(*effect_ptr);

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
    _as = new p_actionSchema(as->name()); // FIXME ensure deletion...
    _as->set_parameters(as->parameters()); // Set parameters makes the copy
    _as->set_precondition(as->precondition().clone());
    _as->set_effect(as->effect().clone());

    const p_Effect* e = &_as->effect();
    setRawEffectPtr(e);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Effects ////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PPDDLInterface::ProbabilisticEffect::ProbabilisticEffect(const PPDDLInterface::p_ProbabilisticEffect *e) : Effect(e) {
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
    return PPDDLInterface::Effect(&constEffect()->effect(i).clone()); // FIXME ensure each clone's deletion!!!!
}

const PPDDLInterface::p_ProbabilisticEffect *PPDDLInterface::ProbabilisticEffect::constEffect() const {
    return static_cast<const p_ProbabilisticEffect*>(_eff);
}

PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::p_ConjunctiveEffect *e): Effect(e) {
    //_ce = const_cast<p_ConjunctiveEffect*>(dynamic_cast<const p_ConjunctiveEffect*>(&e->clone()));
    // We copy it and remove const to be able to successfully modify the effect
}

size_t PPDDLInterface::ConjunctiveEffect::size() const {
    return constEffect()->conjuncts().size();
}

std::shared_ptr<PPDDLInterface::Effect> PPDDLInterface::ConjunctiveEffect::getConjunct(size_t i) const {
    const p_Effect* cjt = &constEffect()->conjuncts()[i]->clone(); // FIXME ensure each clone's deletion!!!!

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
    cj_copy[i] = cjct.getEffect();
    modificableEffect()->set_conjuncts(cj_copy);
    // This makes unnecessary copies... Maybe could be optimised?
}

const PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::constEffect() const {
    return  static_cast<const p_ConjunctiveEffect*>(_eff);
    // If it is inside the Conjunctive Effect class we are sure that the cast is okay
}

PPDDLInterface::p_ConjunctiveEffect *PPDDLInterface::ConjunctiveEffect::modificableEffect() const {
    return const_cast<p_ConjunctiveEffect*>(constEffect());
    // We remove const to be able to successfully modify the effect;
}

PPDDLInterface::ConjunctiveEffect::ConjunctiveEffect(const PPDDLInterface::ConjunctiveEffect &e) : PPDDLInterface::Effect(&e._eff->clone()) {
}

PPDDLInterface::Effect::Effect(const PPDDLInterface::p_Effect *e) {
    _eff = e;
}

const PPDDLInterface::p_Effect* PPDDLInterface::Effect::getEffect() const {
    return _eff;
}
