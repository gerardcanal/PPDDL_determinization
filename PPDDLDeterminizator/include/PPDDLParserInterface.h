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
    typedef ppddl_parser::UpdateEffect p_UpdateEffect;
    typedef ppddl_parser::Update p_Update;
    typedef ppddl_parser::ActionSchema p_actionSchema;
    typedef ppddl_parser::RCObject RCObject;
    typedef ppddl_parser::EffectList p_EffectList;
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
       friend class ProbabilisticEffect;
   public:
        /*!
         * Creates an Effect wrapper from a ppddl_parser::Effect
         * @param e Effect to be wrapped
         */
        Effect(const p_Effect* e);

        /*!
         * Copy constructor
         * @param e Effect to be copied
         */
        Effect(const Effect& e);

        virtual ~Effect();

        /*!
         * Returns the wrapped effect.
         * @return The wrapped Effect
         */
        const p_Effect* getEffect() const;
        virtual Effect & operator= (const Effect & other);

       /*!
        * Checks if the effect is determinized (i.e. doesnot have any probabilistic effect).
        * @param effect Effect to be checked
        * @return True if the effect does not contain probabilistic effects, false otherwise
        */
       static bool determinized(const p_Effect &effect);

       /*!
        * Returns the cost (value of the fluent) of an update effect of type increase or decrease which modifies the fluent
        * metric
        * @param metric name of the fluent to returned.
        * @return The value of the fluent associated with the effect
        */
       virtual double getCost(const std::string& metric="reward");

       /*!
        * Sets the value of the fluent with name metric.
        * @param cost  Value to be set
        * @param metric  Name of the fluent that will be modified.
        */
       virtual void setCost(double cost, const string &metric);

       /*!
        * Returns the cost (value of the fluent) of an update effect of type increase or decrease which modifies the fluent
        * metric
        * @param metric name of the fluent to returned.
        * @return The value of the fluent associated with the effect
        */
       virtual p_Update* getCostFunction(const std::string& metric="reward");

       /*!
        * Sets the value of the fluent with name metric.
        * @param cost  Value to be set
        * @param metric  Name of the fluent that will be modified.
        */
       virtual void setCostFunction(const p_Update* up, const string &metric);

       /*!
        * Sets the value of the fluent with name metric.
        * @param cost  Value to be set
        * @param metric  Name of the fluent that will be modified.
        */
       //virtual void setCostFunction(const p_Update * costfun, const string &metric);

       /*!
       * Returns the cost (value of the fluent) of an update effect of type increase or decrease which modifies the fluent
       * metric
       * @param metric name of the fluent to returned.
       * @return The value of the fluent associated with the effect
       */
       static double getCost(const p_Effect &effect, const std::string& metric);

       /*!
        * Returns the cost (value of the fluent) of an update effect of type increase or decrease which modifies the fluent
        * metric
        * @param metric name of the fluent to returned.
        * @return The value of the fluent associated with the effect
        */
       static p_Update* getCostFunction(const p_Effect &effect, const std::string& metric);

       /*!
       * Sets the value of the fluent with name metric.
       * @param cost  Value to be set
       * @param metric  Name of the fluent that will be modified.
       */
       static void setCost(const p_Effect &effect, double cost, const string &metric);

       /*!
       * Sets the value of the fluent with name metric.
       * @param cost  Value to be set
       * @param metric  Name of the fluent that will be modified.
       */
       static void setCostFunction(const p_Effect &effect, const p_Update* costfun, const string &metric);

       bool probabilitic();
       bool conjunctive();
   protected:
       const p_Effect* _eff; //!< Wrapped effect
       bool _delete_ptr; //!< if true, the pointer will be deleted.
       void releasePtr(); //!< Releases the pointer, thus not deleting it (setting _delete_ptr to false).
   };

    /*!
     * SharedPointer to Effect class
     */
    typedef std::shared_ptr<Effect> EffectPtr;
    EffectPtr makePtr(const Effect& e);

    /*!
     * \class ConjunctiveEffect
     * \brief Wrapper class for the pddl_parser::ConjunctiveEffect class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class ConjunctiveEffect : public Effect {
    public:
        /*!
         * Creates an Effect wrapper from a ppddl_parser::ConjunctiveEffect
         * @param e Effect to be wrapped
         */
        ConjunctiveEffect(const p_ConjunctiveEffect* e);

        /*!
         * Copy constructor for a ConjunctiveEffect
         * @param e Effect to be copied
         */
        ConjunctiveEffect(const PPDDLInterface::ConjunctiveEffect& e);

        size_t size() const; //!< Number of conjuncts of the effect
        std::shared_ptr<Effect> getConjunct(size_t i) const; //!< Returns a copy of the conjunct at the position i

        /*!
         * Changes the conjunct at position i by the cjct parameter
         * @param cjct Conjunct to be added
         * @param i Position at whihc the conjunct will be inserted (*replacing* the existing one)
         */
        void changeConjunct(const Effect& cjct, size_t i);
    private:
        inline const p_ConjunctiveEffect* constEffect() const; //!< Returns the conjunctive effect (const*)
        inline p_ConjunctiveEffect* modificableEffect() const; //!< Returns the conjunctive effect (non-const raw pointer, modifiable).

    };
    EffectPtr makePtr(const ConjunctiveEffect& e);

    /*!
     * \class ProbabilisticEffect
     * \brief Wrapper class for the pddl_parser::ProbabilisticEffect class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class ProbabilisticEffect : public Effect {
    public:
        /*!
         * Creates the wrapper to the ppddl_parser::ProbabilisticEffect
         * @param e Effect to be wraped.
         */
        ProbabilisticEffect(const p_ProbabilisticEffect* e);

        size_t size() const; //!< Returns the number of probabilistic entries of the effect
        double getProbability(size_t i) const; //!< Returns the probability of the ith effect
        void setProbability(double p, size_t i); //!< Sets the probability p of the ith effect
        Effect getEffect(size_t i) const; //!< Returns the ith effect
    private:
        inline const p_ProbabilisticEffect* constEffect() const; //!< Returns the unwrapped effect (const*)
        inline p_ProbabilisticEffect* modificableEffect() const; //!< Returns the unwrapped effect (non-const raw pointer, modifiable).

    };
    EffectPtr makePtr(const ProbabilisticEffect& e);

    /*!
     * class EffectList
     * \brief Encapsulates a list of effects and inherits from the Effect class. This way, the determinizer can return
     * multiple effects which results of the determinization of one effect (which will be converted to multiple actions
     * in the domain).
     */
    class EffectList : public Effect {
    public:
        EffectList();
        EffectList(size_t n);
        EffectList(const EffectList& ef);
        EffectList(const ProbabilisticEffect& pe);
        ~EffectList() = default;
        //EffectList(const EffectList& pe);

        /*!
         * Adds an effect
         * @param e Effect to be added
         * @param w Weight of the effect (probability)
         */
        void addEffect(const Effect& e, double w=1.0);

        std::shared_ptr<Effect> getEffect(size_t i) { return _effects[i]; };
        double getWeight(size_t i) { return _weights[i]; };
        size_t size() const ;
    private:
        std::vector<std::shared_ptr<Effect>> _effects; //!< List of effects
        std::vector<double> _weights; //!< Weights associated with each efefct
    };
    EffectPtr makePtr(const EffectList& e);

    //class Action
    /*!
     * \class Action
     * \brief Wrapper class for the pddl_parser::ActionSchema class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class Action { // TODO should allow r/w access to preconditions...?
        friend class Domain;
    public:
        /*!
         * Creates a ppddl_parser::ActionSchema wrapper
         * @param as Wrapped ActionSchema
         */
        explicit Action(const p_actionSchema* as, const std::string& name_suffix="");

        /*!
         * Copy constructor
         * @param a Action to be copied
         */
        Action(const Action& a, const std::string& name_suffix="");
        Action();
        virtual ~Action();

        /*!
         * Returns the effect.
         * @return Effect of the Action
         */
        std::shared_ptr<Effect> getEffect() const; // Return a pointer because it'd truncate the class to the superclass.

        /*!
         * Sets the effect
         * @param e Effect to set
         */
        void setEffect(const PPDDLInterface::Effect& e); //!< Sets the effect
        std::string getName() const; //!< Returns the name of the action
        Action & operator= (const Action & other);

        double getCost(const std::string& metric);
        void setCost(double cost, const string &metric);

        p_Update* getCostFunction(const std::string& metric);
        void setCostFunction(const p_Update* up, const string &metric);
    protected:
        p_actionSchema* _as; // Wrapped actionSchema
        bool _delete_actionschema; //!< True if the pointer needs to be deleted, false otherwise
        PPDDLInterface::EffectPtr _action_effect; //!< Effect of the _as actionSchema.
                                                // Stored as a pointer to the wrapper to ease the getEffect action.
        void setRawEffectPtr(const p_Effect *e);
        void releasePtr(); //!< Releases the pointer - sets the delete to false.
        void initFrom(const p_actionSchema* as, const std::string& name_suffix=""); //!< Initializes - COPIES!- this action from the parameter
    };

    /*!
     * SharedPointer to Effect class
     */
    typedef std::shared_ptr<Action> ActionPtr;
    ActionPtr makePtr(const Action& e);

    class ActionList : public Action {
    public:
        ActionList();
        ActionList(size_t n);
        ActionList(const ActionList& al);

        /*!
         * Adds an effect
         * @param e Effect to be added
         * @param w Weight of the effect
         */
        void addAction(const Action& a);

        std::shared_ptr<Action> getAction(size_t i) { return _actions[i]; };
        //double getWeight(size_t i) { return _weights[i]; };
        size_t size() const ;
    private:
        std::vector<std::shared_ptr<Action>> _actions; //!< List of actions
    };
    ActionPtr makePtr(const ActionList& e);

    // Domain class
    /*!
     * \class Domain
     * \brief Wrapper class for the pddl_parser::Domain class. It handles the memory and ensures there are no errors nor memory leaks.
     */
    class Domain {
        public:
            typedef std::vector<PPDDLInterface::Action>::iterator action_iterator;

            /*!
             * Creates a domain from the PPDDL file found in domain_path.
             * @param domain_path  Path to the PPDDL file to be read.
             * @param problem_paths Paths to PPDDL problems to be read.
             */
            explicit Domain(const std::string &domain_path, const std::vector<std::string> &problem_paths = std::vector<std::string>()); // Read domain

            /*!
             * Copy constructor
             * @param p Domain to be copied
             * @param name_suffix Suffix that is added to the domain name (as there can not be two domains with the same name in memory)
             */
            Domain(const PPDDLInterface::Domain &p, const std::string &name_suffix = "copy"); // Copy constructor -from a PPDDL domain-
            ~Domain();

            /*!
             * Loads all the problems in the defined paths.
             * @param problem_paths Problems to be read
             */
            void loadProblems(const std::vector<std::string>& problem_paths);

            /*!
             * Get an action from the domain
             * @param name Action name
             * @return The action from the domain
             */
            PPDDLInterface::Action getAction(const std::string& name);

            /*!
             * Get *all* the actions from the domain.
             * @return The list of actions.
             */
            std::vector<PPDDLInterface::Action> getActions() const;

            /*!
             * Set an action to the domain.
             * @param action Action to be set.
             */
            void setAction(const PPDDLInterface::Action& action);

            /*!
             * Prints the domain in PDDL. It must be determinized (i.e. can't contain proabilistic actions) before calling it.
             * @param output_folder_path Folder in which the domain and problem files will be written.
             */
            void printPDDL(const string &output_folder_path, string domain_name="", string problem_name="");


            /*!
             * Prints the domain in PPDDL.
             * @param output_folder_path Folder in which the domain and problem files will be written.
             */
            void printPPDDL(const string &output_folder_path, std::string domain_name="");

            void deleteAction(Action &action);

            /*!
             * Returns a string of type "+/-name". + means the metric is maximize, - minimize. The name represents the
             * name of the cost functions. i.e. "+reward", "-total-cost". An empty string is returned if not found.
             * @return The defined metric, empty string if not found. The metric is looked in the first parsed problem file.
             */
            static std::string getMetric();

            std::string getName();

            std::shared_ptr<p_Domain> _getWrappedDomain() { return _dom; }

    private:
            std::shared_ptr<p_Domain> _dom; //!> Pointer to the domain element.
            bool determinized(); //!> Returns true it the domain is determinized

            /*!
             * Parses a PPDDL file
             * @param path Path to the domain or problem file.
             * @param verbosity  Verbosity level of the parser
             * @param warning_level Warning level of the parser
             * @return True if the domain was parser correctly without errors
             */
            bool readPPDDLFile(const std::string &path, int verbosity = 2, int warning_level = 1);

            std::shared_ptr<VALDomain> getVALDomain(); //!>Returns the VALDomain wrapper of the deterministic version of the domain

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
