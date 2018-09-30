/*
 * Copyright 2003-2005 Carnegie Mellon University and Rutgers University
 * Copyright 2007 H�kan Younes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "effects.h"
#include <cstdlib>
#include <stack>
#include <stdexcept>
#include <typeinfo>


/* ====================================================================== */
/* Update */

namespace ppddl_parser {

/* Constructs an update. */
    Update::Update(const Fluent &fluent, const Expression &expr)
            : fluent_(&fluent), expr_(&expr) {
        RCObject::ref(fluent_);
        RCObject::ref(expr_);
    }


/* Deletes this update. */
    Update::~Update() {
        RCObject::destructive_deref(fluent_);
        RCObject::destructive_deref(expr_);
    }

/* Output operator for updates. */
    std::ostream &operator<<(std::ostream &os, const Update &u) {
        u.print(os);
        return os;
    }

    void Update::setExpression(const Expression *expr) {
        RCObject::destructive_deref(expr_);
        RCObject::ref(expr);
        expr_ = expr;
    }



/* ====================================================================== */
/* Assign */

/* Changes the given state according to this update. */
    void Assign::affect(ValueMap &values) const {
        values[&fluent()] = expression().value(values);
    }


/* Returns an instantiaion of this update. */
    const Update &Assign::instantiation(const SubstitutionMap &subst,
                                        const ValueMap &values) const {
        return *new Assign(fluent().substitution(subst),
                           expression().instantiation(subst, values));
    }


/* Prints this object on the given stream. */
    void Assign::print(std::ostream &os) const {
        os << "(assign " << fluent() << ' ' << expression() << ")";
    }

    const Update &Assign::clone() const {
        return *new Assign(fluent(), expression());
    }

    void Assign::writePPDDL(std::ostream &o) const {
        o << "(assign "; fluent().writePPDDL(o); o << ' '; expression().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* ScaleUp */

/* Changes the given state according to this update. */
    void ScaleUp::affect(ValueMap &values) const {
        ValueMap::const_iterator vi = values.find(&fluent());
        if (vi == values.end()) {
            throw std::logic_error("changing undefined value");
        } else {
            values[&fluent()] = (*vi).second * expression().value(values);
        }
    }


/* Returns an instantiaion of this update. */
    const Update &ScaleUp::instantiation(const SubstitutionMap &subst,
                                         const ValueMap &values) const {
        return *new ScaleUp(fluent().substitution(subst),
                            expression().instantiation(subst, values));
    }

/* Prints this object on the given stream. */
    void ScaleUp::print(std::ostream &os) const {
        os << "(scale-up " << fluent() << ' ' << expression() << ")";
    }

    const Update &ScaleUp::clone() const {
        return *new ScaleUp(fluent(), expression());
    }

    void ScaleUp::writePPDDL(std::ostream &o) const {
        o << "(scale-up "; fluent().writePPDDL(o); o << ' '; expression().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* ScaleDown */

/* Changes the given state according to this update. */
    void ScaleDown::affect(ValueMap &values) const {
        ValueMap::const_iterator vi = values.find(&fluent());
        if (vi == values.end()) {
            throw std::logic_error("changing undefined value");
        } else {
            values[&fluent()] = (*vi).second / expression().value(values);
        }
    }


/* Returns an instantiaion of this update. */
    const Update &ScaleDown::instantiation(const SubstitutionMap &subst,
                                           const ValueMap &values) const {
        return *new ScaleDown(fluent().substitution(subst),
                              expression().instantiation(subst, values));
    }


/* Prints this object on the given stream. */
    void ScaleDown::print(std::ostream &os) const {
        os << "(scale-down " << fluent() << ' ' << expression() << ")";
    }

    const Update &ScaleDown::clone() const {
        return *new ScaleDown(fluent(), expression());
    }

    void ScaleDown::writePPDDL(std::ostream &o) const {
        o << "(scale-down "; fluent().writePPDDL(o); o << ' '; expression().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* Increase */

/* Changes the given state according to this update. */
    void Increase::affect(ValueMap &values) const {
        ValueMap::const_iterator vi = values.find(&fluent());
        if (vi == values.end()) {
            throw std::logic_error("changing undefined value");
        } else {
            values[&fluent()] = (*vi).second + expression().value(values);
        }
    }


/* Returns an instantiaion of this update. */
    const Update &Increase::instantiation(const SubstitutionMap &subst,
                                          const ValueMap &values) const {
        return *new Increase(fluent().substitution(subst),
                             expression().instantiation(subst, values));
    }


/* Prints this object on the given stream. */
    void Increase::print(std::ostream &os) const {
        os << "(increase " << fluent() << ' ' << expression() << ")";
    }

    const Update &Increase::clone() const {
        return *new Increase(fluent(), expression());
    }

    void Increase::writePPDDL(std::ostream &o) const {
        o << "(increase "; fluent().writePPDDL(o); o << ' '; expression().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* Decrease */

/* Changes the given state according to this update. */
    void Decrease::affect(ValueMap &values) const {
        ValueMap::const_iterator vi = values.find(&fluent());
        if (vi == values.end()) {
            throw std::logic_error("changing undefined value");
        } else {
            values[&fluent()] = (*vi).second - expression().value(values);
        }
    }


/* Returns an instantiaion of this update. */
    const Update &Decrease::instantiation(const SubstitutionMap &subst,
                                          const ValueMap &values) const {
        return *new Decrease(fluent().substitution(subst),
                             expression().instantiation(subst, values));
    }


/* Prints this object on the given stream. */
    void Decrease::print(std::ostream &os) const {
        os << "(decrease " << fluent() << ' ' << expression() << ")";
    }

    const Update &Decrease::clone() const {
        return *new Decrease(fluent(), expression());
    }

    void Decrease::writePPDDL(std::ostream &o) const {
        o << "(decrease "; fluent().writePPDDL(o); o << ' '; expression().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* EmptyEffect */

/*
 * An empty effect.
 */
    struct EmptyEffect : public Effect {
        /* Fills the provided lists with a sampled state change for this
           effect in the given state. */
        virtual void state_change(AtomList &adds, AtomList &deletes,
                                  UpdateList &updates,
                                  const TermTable &terms,
                                  const AtomSet &atoms,
                                  const ValueMap &values) const {}

        /* Returns an instantiation of this effect. */
        virtual const Effect &instantiation(const SubstitutionMap &subst,
                                            const TermTable &terms,
                                            const AtomSet &atoms,
                                            const ValueMap &values) const {
            return *this;
        }

        virtual const Effect &clone() const {
            return *this;
        }

    protected:
        /* Prints this object on the given stream. */
        virtual void print(std::ostream &os) const { os << "(and)"; }

    private:
        /* Constant representing the empty effect. */
        static const EmptyEffect EMPTY_;

        /* Constructs an empty effect. */
        EmptyEffect() { ref(this); }

        friend struct Effect;
    };

/* Constant representing the empty effect. */
    const EmptyEffect EmptyEffect::EMPTY_;


/* ====================================================================== */
/* Effect */

/* The empty effect. */
    const Effect &Effect::EMPTY = EmptyEffect::EMPTY_;


/* Conjunction operator for effects. */
    const Effect &operator&&(const Effect &e1, const Effect &e2) {
        if (e1.empty()) {
            return e2;
        } else if (e2.empty()) {
            return e1;
        } else {
            ConjunctiveEffect &conjunction = *new ConjunctiveEffect();
            const ConjunctiveEffect *c1 = dynamic_cast<const ConjunctiveEffect *>(&e1);
            if (c1 != 0) {
                for (EffectList::const_iterator ei = c1->conjuncts().begin();
                     ei != c1->conjuncts().end(); ei++) {
                    conjunction.add_conjunct(**ei);
                }
                RCObject::ref(c1);
                RCObject::destructive_deref(c1);
            } else {
                conjunction.add_conjunct(e1);
            }
            const ConjunctiveEffect *c2 = dynamic_cast<const ConjunctiveEffect *>(&e2);
            if (c2 != 0) {
                for (EffectList::const_iterator ei = c2->conjuncts().begin();
                     ei != c2->conjuncts().end(); ei++) {
                    conjunction.add_conjunct(**ei);
                }
                RCObject::ref(c2);
                RCObject::destructive_deref(c2);
            } else {
                conjunction.add_conjunct(e2);
            }
            return conjunction;
        }
    }


/* Output operator for effects. */
    std::ostream &operator<<(std::ostream &os, const Effect &e) {
        e.print(os);
        return os;
    }

    void Effect::writePPDDL(std::ostream &o) const {
        o << *this;
    }


/* ====================================================================== */
/* SimpleEffect */

/* Constructs a simple effect. */
    SimpleEffect::SimpleEffect(const Atom &atom)
            : atom_(&atom) {
        ref(atom_);
    }

/* Deletes this simple effect. */
    SimpleEffect::~SimpleEffect() {
        destructive_deref(atom_);
    }


/* ====================================================================== */
/* AddEffect */

/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void AddEffect::state_change(AtomList &adds, AtomList &deletes,
                                 UpdateList &updates,
                                 const TermTable &terms,
                                 const AtomSet &atoms,
                                 const ValueMap &values) const {
        adds.push_back(&atom());
    }

    const Effect &AddEffect::clone() const {
        AddEffect *cpy = new AddEffect(Atom::make(this->atom().predicate(), this->atom().terms()));
        ref(cpy);
        return *cpy;
    }

/* Returns an instantiation of this effect. */
    const Effect &AddEffect::instantiation(const SubstitutionMap &subst,
                                           const TermTable &terms,
                                           const AtomSet &atoms,
                                           const ValueMap &values) const {
        const Atom *inst_atom = &atom().substitution(subst);
        if (inst_atom == &atom()) {
            return *this;
        } else {
            return *new AddEffect(*inst_atom);
        }
    }


/* Prints this object on the given stream. */
    void AddEffect::print(std::ostream &os) const {
        os << atom();
    }

    void AddEffect::writePPDDL(std::ostream &o) const {
        print(o);
    }


/* ====================================================================== */
/* DeleteEffect */

/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void DeleteEffect::state_change(AtomList &adds, AtomList &deletes,
                                    UpdateList &updates,
                                    const TermTable &terms,
                                    const AtomSet &atoms,
                                    const ValueMap &values) const {
        deletes.push_back(&atom());
    }


/* Returns an instantiation of this effect. */
    const Effect &DeleteEffect::instantiation(const SubstitutionMap &subst,
                                              const TermTable &terms,
                                              const AtomSet &atoms,
                                              const ValueMap &values) const {
        const Atom *inst_atom = &atom().substitution(subst);
        if (inst_atom == &atom()) {
            return *this;
        } else {
            return *new DeleteEffect(*inst_atom);
        }
    }

    const Effect &DeleteEffect::clone() const {
        DeleteEffect *cpy = new DeleteEffect(Atom::make(this->atom().predicate(), this->atom().terms()));
        ref(cpy);
        return *cpy;
    }

/* Prints this object on the given stream. */
    void DeleteEffect::print(std::ostream &os) const {
        os << "(not " << atom() << ")";
    }

    void DeleteEffect::writePPDDL(std::ostream &o) const {
        print(o);
    }


/* ====================================================================== */
/* UpdateEffect */

/* Returns an effect for the given update. */
    const Effect &UpdateEffect::make(const Update &update) {
        if (typeid(update) == typeid(ScaleUp)
            || typeid(update) == typeid(ScaleDown)) {
            const Value *v = dynamic_cast<const Value *>(&update.expression());
            if (v != 0 && v->value() == 1) {
                return EMPTY;
            }
        } else if (typeid(update) == typeid(Increase)
                   || typeid(update) == typeid(Decrease)) {
            const Value *v = dynamic_cast<const Value *>(&update.expression());
            if (v != 0 && v->value() == 0) {
                return EMPTY;
            }
        }
        return *new UpdateEffect(update);
    }


/* Deletes this update effect. */
    UpdateEffect::~UpdateEffect() {
        delete update_;
    }


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void UpdateEffect::state_change(AtomList &adds, AtomList &deletes,
                                    UpdateList &updates,
                                    const TermTable &terms,
                                    const AtomSet &atoms,
                                    const ValueMap &values) const {
        updates.push_back(update_);
    }


/* Returns an instantiation of this effect. */
    const Effect &UpdateEffect::instantiation(const SubstitutionMap &subst,
                                              const TermTable &terms,
                                              const AtomSet &atoms,
                                              const ValueMap &values) const {
        return *new UpdateEffect(update().instantiation(subst, values));
    }

    const Effect &UpdateEffect::clone() const {
        const Effect *cpy = &make(this->update_->clone());
        ref(cpy);
        return *cpy;
    }

/* Prints this object on the given stream. */
    void UpdateEffect::print(std::ostream &os) const {
        os << update();
    }

    void UpdateEffect::writePPDDL(std::ostream &o) const {
        update().writePPDDL(o);
    }


/* ====================================================================== */
/* ConjunctiveEffect */
/* Deletes this conjunctive effect. */
    ConjunctiveEffect::~ConjunctiveEffect() {
        for (EffectList::const_iterator ei = conjuncts().begin();
             ei != conjuncts().end(); ei++) {
            destructive_deref(*ei);
        }
    }

    const Effect &ConjunctiveEffect::clone() const {
        ConjunctiveEffect *cpy = new ConjunctiveEffect();
        ref(cpy);
        cpy->conjuncts_.resize(conjuncts_.size());
        for (size_t i = 0; i < conjuncts_.size(); ++i) {
            cpy->conjuncts_[i] = &this->conjuncts_[i]->clone();
        }
        return *cpy;
    }

/* Adds a conjunct to this conjunctive effect. */
    void ConjunctiveEffect::add_conjunct(const Effect &conjunct) {
        const ConjunctiveEffect *conj_effect =
                dynamic_cast<const ConjunctiveEffect *>(&conjunct);
        if (conj_effect != 0) {
            for (EffectList::const_iterator ei = conj_effect->conjuncts().begin();
                 ei != conj_effect->conjuncts().end(); ei++) {
                conjuncts_.push_back(*ei);
                ref(*ei);
            }
            ref(&conjunct);
            destructive_deref(&conjunct);
        } else {
            conjuncts_.push_back(&conjunct);
            ref(&conjunct);
        }
    }


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void ConjunctiveEffect::state_change(AtomList &adds, AtomList &deletes,
                                         UpdateList &updates,
                                         const TermTable &terms,
                                         const AtomSet &atoms,
                                         const ValueMap &values) const {
        for (EffectList::const_iterator ei = conjuncts().begin();
             ei != conjuncts().end(); ei++) {
            (*ei)->state_change(adds, deletes, updates, terms, atoms, values);
        }
    }


/* Returns an instantiation of this effect. */
    const Effect &ConjunctiveEffect::instantiation(const SubstitutionMap &subst,
                                                   const TermTable &terms,
                                                   const AtomSet &atoms,
                                                   const ValueMap &values) const {
        ConjunctiveEffect &inst_effect = *new ConjunctiveEffect();
        for (EffectList::const_iterator ei = conjuncts().begin();
             ei != conjuncts().end(); ei++) {
            inst_effect.add_conjunct((*ei)->instantiation(subst, terms,
                                                          atoms, values));
        }
        return inst_effect;
    }


/* Prints this object on the given stream. */
    void ConjunctiveEffect::print(std::ostream &os) const {
        os << "(and";
        for (EffectList::const_iterator ei = conjuncts().begin();
             ei != conjuncts().end(); ei++) {
            os << ' ' << **ei;
        }
        os << ")";
    }

    void ConjunctiveEffect::writePPDDL(std::ostream &o) const {
        o << "(and";
        for (EffectList::const_iterator ei = conjuncts().begin(); ei != conjuncts().end(); ei++) {
            o << ' '; (*ei)->writePPDDL(o);
        }
        o << ")";
    }


/* ====================================================================== */
/* ConditionalEffect */

/* Returns a conditional effect. */
    const Effect &ConditionalEffect::make(const StateFormula &condition,
                                          const Effect &effect) {
        if (condition.tautology()) {
            return effect;
        } else if (condition.contradiction() || effect.empty()) {
            ref(&effect); // There is a memory leak if there are no more references to this effect and the pointer gets lost.
            destructive_deref(
                    &effect); // As the pointer may not be initialized (0 counts), destructive_deref may underflow the counter
            return EMPTY; // Therefore I add 1 and then remove 1 (+ deletion if it was the only reference). This causes no effect if there was more references
        } else {
            return *new ConditionalEffect(condition, effect);
        }
    }


/* Constructs a conditional effect. */
    ConditionalEffect::ConditionalEffect(const StateFormula &condition,
                                         const Effect &effect)
            : condition_(&condition), effect_(&effect) {
        ref(condition_);
        ref(effect_);
    }


/* Deletes this conditional effect. */
    ConditionalEffect::~ConditionalEffect() {
        destructive_deref(condition_);
        destructive_deref(effect_);
    }


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void ConditionalEffect::state_change(AtomList &adds, AtomList &deletes,
                                         UpdateList &updates,
                                         const TermTable &terms,
                                         const AtomSet &atoms,
                                         const ValueMap &values) const {
        if (condition().holds(terms, atoms, values)) {
            /* Effect condition holds. */
            effect().state_change(adds, deletes, updates, terms, atoms, values);
        }
    }

    const Effect &ConditionalEffect::clone() const {
        ConditionalEffect *cpy = new ConditionalEffect(condition_->clone(), effect_->clone());
        ref(cpy);
        deref(cpy->effect_); // As ConditionalEffect() increments ref on effect, and clone() does too.
        return *cpy;
    }

/* Returns an instantiation of this effect. */
    const Effect &ConditionalEffect::instantiation(const SubstitutionMap &subst,
                                                   const TermTable &terms,
                                                   const AtomSet &atoms,
                                                   const ValueMap &values) const {
        return make(condition().instantiation(subst, terms, atoms, values, false),
                    effect().instantiation(subst, terms, atoms, values));
    }


/* Prints this object on the given stream. */
    void ConditionalEffect::print(std::ostream &os) const {
        os << "(when " << condition() << ' ' << effect() << ")";
    }

    void ConditionalEffect::writePPDDL(std::ostream &o) const {
        o << "(when " << condition() << ' '; effect().writePPDDL(o); o << ")";
    }


/* ====================================================================== */
/* ProbabilisticEffect */

/* Returns a probabilistic effect. */
    const Effect &
    ProbabilisticEffect::make(const std::vector<std::pair<Rational,
            const Effect *> > &os) {
        for (size_t i = 0; i < os.size(); i++) {
            if (os[i].first > 0 && !os[i].second->empty()) {
                ProbabilisticEffect &peff = *new ProbabilisticEffect();
                for (; i < os.size(); i++) {
                    peff.add_outcome(os[i].first, *os[i].second);
                }
                return peff;
            }
        }
        return EMPTY;
    }


/* Deletes this probabilistic effect. */
    ProbabilisticEffect::~ProbabilisticEffect() {
        for (EffectList::const_iterator ei = effects_.begin();
             ei != effects_.end(); ei++) {
            destructive_deref(*ei);
        }
    }


/* Adds an outcome to this probabilistic effect. */
    bool ProbabilisticEffect::add_outcome(const Rational &p,
                                          const Effect &effect) {
        const ProbabilisticEffect *prob_effect =
                dynamic_cast<const ProbabilisticEffect *>(&effect);
        if (prob_effect != 0) {
            size_t n = prob_effect->size();
            for (size_t i = 0; i < n; i++) {
                if (!add_outcome(p * prob_effect->probability(i),
                                 prob_effect->effect(i))) {
                    return false;
                }
            }
            ref(&effect);
            destructive_deref(&effect);
        } else if (p != 0) {
            effects_.push_back(&effect);
            ref(&effect);
            if (weight_sum_ == 0) {
                weights_.push_back(p.numerator());
                weight_sum_ = p.denominator();
                return true;
            } else {
                std::pair<int, int> m =
                        Rational::multipliers(weight_sum_, p.denominator());
                int sum = 0;
                size_t n = size();
                for (size_t i = 0; i < n; i++) {
                    sum += weights_[i] *= m.first;
                }
                weights_.push_back(p.numerator() * m.second);
                sum += p.numerator() * m.second;
                weight_sum_ *= m.first;
                return sum <= weight_sum_;
            }
        }
        return true;
    }


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void ProbabilisticEffect::state_change(AtomList &adds, AtomList &deletes,
                                           UpdateList &updates,
                                           const TermTable &terms,
                                           const AtomSet &atoms,
                                           const ValueMap &values) const {
        if (size() != 0) {
            int w = int(rand() / (RAND_MAX + 1.0) * weight_sum_);
            int wtot = 0;
            size_t n = size();
            for (size_t i = 0; i < n; i++) {
                wtot += weights_[i];
                if (w < wtot) {
                    effect(i).state_change(adds, deletes, updates, terms, atoms, values);
                    return;
                }
            }
        }
    }


/* Returns an instantiation of this effect. */
    const Effect &
    ProbabilisticEffect::instantiation(const SubstitutionMap &subst,
                                       const TermTable &terms,
                                       const AtomSet &atoms,
                                       const ValueMap &values) const {
        ProbabilisticEffect &inst_effect = *new ProbabilisticEffect();
        size_t n = size();
        for (size_t i = 0; i < n; i++) {
            inst_effect.add_outcome(probability(i),
                                    effect(i).instantiation(subst, terms,
                                                            atoms, values));
        }
        return inst_effect;
    }

    const Effect &ProbabilisticEffect::clone() const {
        ProbabilisticEffect *cpy = new ProbabilisticEffect();
        ref(cpy);
        cpy->weights_ = this->weights_;
        cpy->weight_sum_ = this->weight_sum_;
        cpy->effects_.resize(this->effects_.size());
        for (size_t i = 0; i < this->effects_.size(); ++i) {
            cpy->effects_[i] = &this->effects_[i]->clone();
        }
        return *cpy;
    }

/* Prints this object on the given stream. */
    void ProbabilisticEffect::print(std::ostream &os) const {
        if (weight_sum_ == 0) {
            os << "(and)";
        } else if (weight_sum_ == weights_.back()) {
            os << effect(0);
        } else {
            os << "(probabilistic";
            size_t n = size();
            for (size_t i = 0; i < n; i++) {
                os << ' ' << probability(i) << ' ' << effect(i);
            }
            os << ")";
        }
    }

    void ProbabilisticEffect::writePPDDL(std::ostream &o) const {
        if (weight_sum_ == 0) {
            o << "(and)";
        } else if (weight_sum_ == weights_.back()) {
            o << effect(0);
        } else {
            o << "(probabilistic";
            size_t n = size();
            for (size_t i = 0; i < n; i++) {
                o << ' ' << probability(i).double_value() << ' ' << effect(i);
            }
            o << ")";
        }
    }


/* ====================================================================== */
/* QuantifiedEffect */

/* Returns a universally quantified effect. */
    const Effect &QuantifiedEffect::make(const VariableList &parameters,
                                         const Effect &effect) {
        if (parameters.empty() || effect.empty()) {
            return effect;
        } else {
            return *new QuantifiedEffect(parameters, effect);
        }
    }


/* Constructs a universally quantified effect. */
    QuantifiedEffect::QuantifiedEffect(const VariableList &parameters,
                                       const Effect &effect)
            : parameters_(parameters), effect_(&effect) {
        ref(effect_);
    }


/* Deletes this universally quantifed effect. */
    QuantifiedEffect::~QuantifiedEffect() {
        destructive_deref(effect_);
    }


/* Fills the provided lists with a sampled state change for this
   effect in the given state. */
    void QuantifiedEffect::state_change(AtomList &adds, AtomList &deletes,
                                        UpdateList &updates,
                                        const TermTable &terms,
                                        const AtomSet &atoms,
                                        const ValueMap &values) const {
        throw std::logic_error("Quantified::state_change not implemented");
    }

    const Effect &QuantifiedEffect::clone() const {
        QuantifiedEffect *cpy = new QuantifiedEffect(parameters_, effect_->clone());
        destructive_deref(cpy->effect_); // As both clone and the constructor QuantifiedEffect increments ref!
        ref(cpy);
        return *cpy;
    }

/* Returns an instantiation of this effect. */
    const Effect &QuantifiedEffect::instantiation(const SubstitutionMap &subst,
                                                  const TermTable &terms,
                                                  const AtomSet &atoms,
                                                  const ValueMap &values) const {
        int n = parameters().size();
        if (n == 0) {
            return effect().instantiation(subst, terms, atoms, values);
        } else {
            SubstitutionMap args(subst);
            std::vector<const ObjectList *> arguments(n);
            std::vector<ObjectList::const_iterator> next_arg;
            for (int i = 0; i < n; i++) {
                Type t = TermTable::type(parameters()[i]);
                arguments[i] = &terms.compatible_objects(t);
                if (arguments[i]->empty()) {
                    for (int i = 0; i < n; i++) delete arguments[i]; // Fix memory leak! Objectlists were never deleted
                    return EMPTY;
                }
                next_arg.push_back(arguments[i]->begin());
            }
            const Effect *conj = &EMPTY;
            std::stack<const Effect *> conjuncts;
            conjuncts.push(&effect().instantiation(args, terms, atoms, values));
            ref(conjuncts.top());
            for (int i = 0; i < n;) {
                SubstitutionMap pargs;
                pargs.insert(std::make_pair(parameters()[i], *next_arg[i]));
                const Effect &conjunct =
                        conjuncts.top()->instantiation(pargs, terms, atoms, values);
                conjuncts.push(&conjunct);
                if (i + 1 == n) {
                    conj = &(*conj && conjunct);
                    for (int j = i; j >= 0; j--) {
                        if (j < i) {
                            destructive_deref(conjuncts.top());
                        }
                        conjuncts.pop();
                        next_arg[j]++;
                        if (next_arg[j] == arguments[j]->end()) {
                            if (j == 0) {
                                i = n;
                                break;
                            } else {
                                next_arg[j] = arguments[j]->begin();
                            }
                        } else {
                            i = j;
                            break;
                        }
                    }
                } else {
                    ref(conjuncts.top());
                    i++;
                }
            }
            while (!conjuncts.empty()) {
                destructive_deref(conjuncts.top());
                conjuncts.pop();
            }
            for (int i = 0; i < n; i++) delete arguments[i]; // Fix memory leak! Objectlists were never deleted
            return *conj;
        }
    }


/* Prints this object on the given stream. */
    void QuantifiedEffect::print(std::ostream &os) const {
        if (parameters().empty()) {
            os << effect();
        } else {
            VariableList::const_iterator vi = parameters().begin();
            os << "(forall (" << *vi;
            for (vi++; vi != parameters().end(); vi++) {
                os << ' ' << *vi;
            }
            os << ") " << effect() << ")";
        }
    }

    void QuantifiedEffect::writePPDDL(std::ostream &o) const {
        if (parameters().empty()) {
            effect().writePPDDL(o);
        } else {
            VariableList::const_iterator vi = parameters().begin();
            o << "(forall (" << *vi;
            for (vi++; vi != parameters().end(); vi++) {
                o << ' ' << *vi;
            }
            o << ") "; effect().writePPDDL(o); o << ")";
        }
    }

}