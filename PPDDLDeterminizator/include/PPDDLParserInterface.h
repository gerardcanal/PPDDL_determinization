//
// Created by gcanal on 18/12/17.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H


/**
 * THIS CLASSES ARE AN INTERFACE TO THE MDPSIM PARSER CLASSES, HANDLE THE MEMORY AND EASENS THE INTERFACE. THESE ARE THE
 * ONLY ONES THAT SHOULD BE USED TO INTERACT WITH THE PLANNING DOMAIN!
 */

#include "PPDDLParser/domains.h"

// Domain class
namespace PPDDLInterface {

// Typedefs
    typedef ::Domain p_Domain; // The ::Domain syntax makes it reference to the upper scope namespace i.e. to the MDPSim parser one in this case
    typedef ::Effect p_Effect;
    typedef ::ConjunctiveEffect p_ConjunctiveEffect;
    typedef ::ProbabilisticEffect p_ProbabilisticEffect;
    typedef ::ActionSchema p_actionSchema;
// Classes
    //Generic class Effect
   class Effect {
    public:
        Effect(const p_Effect* e);
    private:
        const p_Effect* _eff;
    };

    class ConjunctiveEffect : public Effect {
    public:
        ConjunctiveEffect(const p_ConjunctiveEffect* e);
    private:
        const p_ConjunctiveEffect* _ce;
    };

    class ProbabilisticEffect : public Effect {
    public:
        ProbabilisticEffect(const p_ProbabilisticEffect* e);
    private:
        const p_ProbabilisticEffect* _pe;
    };


    //class Action
    class Action {
    public:
        explicit Action(const p_actionSchema* as);
        ~Action();

        PPDDLInterface::Effect getEffect();
    private:
        const p_actionSchema* _as; // Wrapped actionSchema
        PPDDLInterface::Effect *_action_effect; // Effect of the _as actionSchema.
        // Stored as a pointer to the wrapper to ease the getEffect action.


        void setEffect(const PPDDLInterface::Effect& e);
    };

    class Domain { // TODO copy constructor initialization from a p_Domain?
        public:
            explicit Domain(const std::string& domain_path); // Read domain
            Domain(const PPDDLInterface::Domain& p); // Copy constructor -from a PPDDL domain-
            ~Domain();

            PPDDLInterface::Action getAction(const std::string& name);
        private:
             p_Domain* _dom;

            bool readDomain(const std::string &domain_path, int verbosity=2, int warning_level=1);


            friend std::ostream &operator<<(std::ostream &output, const Domain &D) {
                output << *D._dom;
                return output;
            }
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Definitions for the parser /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The parse function. */
extern int ppddl_parse(); // FIXME namespace this variables!?
/* File to parse. */
extern FILE* yyin;
#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
