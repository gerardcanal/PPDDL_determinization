//
// Created by gcanal on 18/12/17.
//

#ifndef ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
#define ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H


/**
 * THIS CLASSES ARE AN INTERFACE TO THE MDPSIM PARSER CLASSES, HANDLE THE MEMORY AND EASENS THE INTERACTION WITH THEM.
 * NOTE THESE ARE THE ONLY ONES THAT SHOULD BE USED TO INTERACT WITH THE PLANNING DOMAIN!
 */


#include "PPDDLParser/domains.h"
#include <memory>
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
       friend class Action;
       friend class ConjunctiveEffect;
    public:
        Effect(const p_Effect* e);
       virtual ~Effect() = default;
    //FIXME protected:
        const p_Effect* _eff;
    };

    class ConjunctiveEffect : public Effect {
    public:
        ConjunctiveEffect(const p_ConjunctiveEffect* e);

        size_t size() const;
        std::shared_ptr<Effect> getConjunct(size_t i) const;
        void changeConjunct(const Effect& cjct, size_t i) const;
    private:
        p_ConjunctiveEffect* _ce;
    };

    class ProbabilisticEffect : public Effect {
    public:
        ProbabilisticEffect(const p_ProbabilisticEffect* e);

        size_t size() const;
        double getProbability(size_t i) const;
        Effect getEffect(size_t i) const;
    private:
        p_ProbabilisticEffect* _pe;
    };


    //class Action
    class Action { // FIXME should allow r/w access to preconditions...?
        friend class Domain;
    public:
        explicit Action(const p_actionSchema* as);
        Action(const Action& a);
        Action();
        ~Action();

        std::shared_ptr<Effect> getEffect() const; // Return a pointer because it'd truncate the class to the superclass.
        void setEffect(const PPDDLInterface::Effect& e);
        inline std::string getName() const;
    private:
        std::shared_ptr<p_actionSchema> _as; // Wrapped actionSchema
        std::shared_ptr<PPDDLInterface::Effect> _action_effect; // Effect of the _as actionSchema.
                                                // Stored as a pointer to the wrapper to ease the getEffect action.
    };

    class Domain {
        public:
            typedef std::vector<PPDDLInterface::Action>::iterator action_iterator;


            explicit Domain(const std::string& domain_path); // Read domain
            Domain(const PPDDLInterface::Domain& p); // Copy constructor -from a PPDDL domain-
            ~Domain();

            PPDDLInterface::Action getAction(const std::string& name);
            std::vector<PPDDLInterface::Action> getActions();

            void setAction(const PPDDLInterface::Action& action);
        private:
             std::shared_ptr<p_Domain> _dom;

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
