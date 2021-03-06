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
#include "requirements.h"


/* ====================================================================== */
/* Requirements */

namespace ppddl_parser {

/* Constructs a default requirements object. */
    Requirements::Requirements()
            : strips(true), typing(false), negative_preconditions(false),
              disjunctive_preconditions(false), equality(false),
              existential_preconditions(false), universal_preconditions(false),
              conditional_effects(false), fluents(false), probabilistic_effects(false),
              rewards(false) {}


/* Enables quantified preconditions. */
    void Requirements::quantified_preconditions() {
      existential_preconditions = true;
      universal_preconditions = true;
    }


/* Enables ADL style actions. */
    void Requirements::adl() {
      strips = true;
      typing = true;
      negative_preconditions = true;
      disjunctive_preconditions = true;
      equality = true;
      quantified_preconditions();
      conditional_effects = true;
    }


/* Enables MDP planning problems. */
    void Requirements::mdp() {
      probabilistic_effects = true;
      rewards = true;
    }

    void Requirements::writePPDDL(std::ostream &o) const {
        if (strips) o << " :strips";
        if (typing) o << " :typing";
        if (negative_preconditions) o << " :negative-preconditions";
        if (disjunctive_preconditions) o << " :disjunctive-preconditions";
        if (equality) o << " :equality";
        if (existential_preconditions) o << " :existential-preconditions";
        if (universal_preconditions) o << " :universal-preconditions";
        if (conditional_effects) o << " :conditional-effects";
        if (fluents) o << " :fluents";
        if (probabilistic_effects) o << " :probabilistic-effects";
        if (rewards) o << " :rewards";
    }
}