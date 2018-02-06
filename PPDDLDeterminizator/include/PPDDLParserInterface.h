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
#include "PPDDLParser/problems.h"
#include "VALConversion.h"
#include "ptree.h"
#include "PDDLPrinter.h"
#include <memory>
#include <fstream>

namespace PPDDLInterface {
// Typedefs
    typedef ppddl_parser::Domain p_Domain; // The ::Domain syntax makes it reference to the upper scope namespace i.e. to the MDPSim parser one in this case
    typedef ppddl_parser::Effect p_Effect;
    typedef ppddl_parser::ConjunctiveEffect p_ConjunctiveEffect;
    typedef ppddl_parser::ProbabilisticEffect p_ProbabilisticEffect;
    typedef ppddl_parser::ActionSchema p_actionSchema;
    typedef ppddl_parser::RCObject RCObject;
    typedef ppddl_parser::EffectList EffectList;
    /*typedef ppddl_parser::TypeTable TypeTable;
    typedef ppddl_parser::PredicateTable PredicateTable;
    typedef ppddl_parser::FunctionTable FunctionTable;
    typedef ppddl_parser::TermTable TermTable;*/

// Classes
    //Generic class Effect
   /*!
    * \class Effect
    * \brief Wrapper class for the pddl_parser::Effect class. It handles the memory and ensures there are no errors nor memory leaks.
    */
   class Effect {
       friend class Action;
       friend class ConjunctiveEffect;
       friend class ProabbilisticEffect;
    public:
        /*!
         *
         * @param e
         */
        Effect(const p_Effect* e);
        Effect(const Effect& e);

        virtual ~Effect();
        const p_Effect* getEffect() const;
        virtual Effect & operator= (const Effect & other);

       static bool determinized(const ppddl_parser::Effect &effect);

   protected:
       const p_Effect* _eff;
       bool _delete_ptr;
       void releasePtr();
   };

    /*!
     * \class ConjunctiveEffect
     * \brief Wrapper class for the pddl_parser::ConjunctiveEffect class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class ConjunctiveEffect : public Effect {
    public:
        ConjunctiveEffect(const p_ConjunctiveEffect* e);
        ConjunctiveEffect(const PPDDLInterface::ConjunctiveEffect& e);

        size_t size() const;
        std::shared_ptr<Effect> getConjunct(size_t i) const;
        void changeConjunct(const Effect& cjct, size_t i);
    private:
        inline const p_ConjunctiveEffect* constEffect() const;
        inline p_ConjunctiveEffect* modificableEffect() const;

    };

    /*!
     * \class ProbabilisticEffect
     * \brief Wrapper class for the pddl_parser::ProbabilisticEffect class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class ProbabilisticEffect : public Effect {
    public:
        ProbabilisticEffect(const p_ProbabilisticEffect* e);

        size_t size() const;
        double getProbability(size_t i) const;
        Effect getEffect(size_t i) const;
    private:
        inline const p_ProbabilisticEffect* constEffect() const;
    };


    //class Action
    /*!
     * \class Action
     * \brief Wrapper class for the pddl_parser::ActionSchema class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class Action { // TODO should allow r/w access to preconditions...?
        friend class Domain;
    public:
        explicit Action(const p_actionSchema* as);
        Action(const Action& a);
        Action();
        ~Action();

        std::shared_ptr<Effect> getEffect() const; // Return a pointer because it'd truncate the class to the superclass.
        void setEffect(const PPDDLInterface::Effect& e);
        std::string getName() const;
        Action & operator= (const Action & other);
    private:
        p_actionSchema* _as; // Wrapped actionSchema
        bool _delete_actionschema;
        std::shared_ptr<PPDDLInterface::Effect> _action_effect; // Effect of the _as actionSchema.
                                                // Stored as a pointer to the wrapper to ease the getEffect action.
        void setRawEffectPtr(const p_Effect *e);
        void releasePtr();
        void initFrom(const p_actionSchema* as);
    };


    // Domain class
    /*!
     * \class Domain
     * \brief Wrapper class for the pddl_parser::Domain class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class Domain {
        public:
            typedef std::vector<PPDDLInterface::Action>::iterator action_iterator;

            explicit Domain(const std::string& domain_path); // Read domain
            Domain(const PPDDLInterface::Domain &p, const std::string &name_suffix = "copy"); // Copy constructor -from a PPDDL domain-
            ~Domain();

            PPDDLInterface::Action getAction(const std::string& name);
            std::vector<PPDDLInterface::Action> getActions() const;

            void setAction(const PPDDLInterface::Action& action);
            void printPDDL(const string &output_folder_path);
        private:
            std::shared_ptr<p_Domain> _dom;
            bool determinized(); // FIXME initialize somehow

            bool readDomain(const std::string &domain_path, int verbosity=2, int warning_level=1);

            std::shared_ptr<VALDomain> getVALDomain();

            friend std::ostream &operator<<(std::ostream &output, const Domain &D) {
                output << *D._dom;
                return output;
            }
            friend class VALConversion;
    };
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Definitions for the parser /////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The parse function. */
extern int ppddl_parse();
/* File to parse. */
extern FILE* ppddl_in;
#endif //ROSPLAN_PLANNING_SYSTEM_PPDDLPARSERINTERFACE_H
