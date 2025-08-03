#ifndef TASKS_TSEITIN_TASK_H
#define TASKS_TSEITIN_TASK_H

#include "delegating_task.h"
#include "tseitin_transformer.h" // TseitinAxiom + IntPairHash
#include <unordered_map>

namespace tasks
{

    class TseitinTask : public DelegatingTask
    {
        std::vector<TseitinAxiom> axioms;                                     // alte+neue Axiome
        std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> heads; // (op,eff)→head
        int parent_vars;                                                      // #Variablen im Original-Task
        int aux_vars;                                                         // #erzeugte Hilfs-Variablen

    public:
        TseitinTask(std::shared_ptr<AbstractTask> parent,
                    std::vector<TseitinAxiom> axioms_,
                    std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> heads_,
                    int aux_vars_);

        /* ---------- Variablen ---------- */
        int get_num_variables() const override;
        int get_variable_domain_size(int var) const override;
        int get_variable_axiom_layer(int var) const override;
        int get_variable_default_axiom_value(int var) const override;
        std::vector<int> get_initial_state_values() const override;
        void convert_state_values_from_parent(std::vector<int> &) const override;

        /* ---------- Axiome ---------- */
        int get_num_axioms() const override;
        int get_num_operator_preconditions(int op, bool ax) const override;
        FactPair get_operator_precondition(int op, int idx, bool ax) const override;

        /* ---------- Effekt-Prä-Conditions ---------- */
        int get_num_operator_effect_conditions(int op, int eff, bool ax) const override;
        FactPair get_operator_effect_condition(int op, int eff, int idx, bool ax) const override;

        /* ---------- Debug-Name ---------- */
        std::string get_operator_name(int op, bool ax) const override;
    };

} // namespace tasks
#endif
