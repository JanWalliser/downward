// src/search/tasks/tseitin_task.h

#ifndef TASKS_TSEITIN_TASK_H
#define TASKS_TSEITIN_TASK_H

#include "delegating_task.h"
#include "tseitin_transformer.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace tasks
{

    class TseitinTask : public DelegatingTask
    {
        int parent_vars;
        int aux_vars;
        std::vector<TseitinAxiom> axioms;
        std::unordered_map<FactPair, std::pair<int, int>, FactPairHash> fact_to_pair;

    public:
        TseitinTask(std::shared_ptr<AbstractTask> parent,
                    const std::vector<TseitinAxiom> &axioms_,
                    const std::unordered_map<FactPair, std::pair<int, int>, FactPairHash> &mapping);

        int get_num_variables() const override;
        int get_variable_domain_size(int var) const override;
        int get_variable_axiom_layer(int var) const override;
        int get_variable_default_axiom_value(int var) const override;
        std::vector<int> get_initial_state_values() const override;
        void convert_state_values_from_parent(std::vector<int> &values) const override;

        int get_num_axioms() const override;
        int get_num_operator_preconditions(int op_index, bool is_axiom) const override;
        FactPair get_operator_precondition(int op_index, int cond_index, bool is_axiom) const override;
        int get_num_operator_effects(int op_index, bool is_axiom) const override;
        FactPair get_operator_effect(int op_index, int eff_index, bool is_axiom) const override;
        int get_num_operator_effect_conditions(int op_index, int eff_index, bool is_axiom) const override;
        FactPair get_operator_effect_condition(int op_index, int eff_index, int cond_index, bool is_axiom) const override;
        std::string get_operator_name(int op_index, bool is_axiom) const override;

        std::string get_variable_name(int var) const override;
        std::string get_fact_name(const FactPair &fp) const override;
        bool are_facts_mutex(const FactPair &f1, const FactPair &f2) const override;
    };

} // namespace tasks

#endif // TASKS_TSEITIN_TASK_H
