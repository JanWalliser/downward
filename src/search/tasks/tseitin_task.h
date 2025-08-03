#pragma once

#include "delegating_task.h"
#include "tseitin_transformer.h"
#include <memory>
#include <vector>

namespace tasks
{

    class TseitinTask : public DelegatingTask
    {
    public:
        /**
         * @param parent   Originaltask (echte Operatoren unverändert)
         * @param axioms_  Alle transformierten Domain‐Axiome
         * @param aux_vars Anzahl der neuen Hilfsvariablen
         */
        TseitinTask(std::shared_ptr<AbstractTask> parent,
                    std::vector<TseitinAxiom> axioms_,
                    int aux_vars_);

        int get_num_variables() const override;
        int get_variable_domain_size(int var) const override;
        int get_variable_axiom_layer(int var) const override;
        int get_variable_default_axiom_value(int var) const override;
        std::vector<int> get_initial_state_values() const override;
        void convert_state_values_from_parent(std::vector<int> &values) const override;

        int get_num_operators() const override { return parent->get_num_operators(); }
        std::string get_operator_name(int index, bool is_axiom) const override;

        // --- Tseitin‐Axiome ---
        int get_num_axioms() const override { return static_cast<int>(axioms.size()); }

        int get_num_operator_preconditions(int index, bool is_axiom) const override;
        FactPair get_operator_precondition(int op_index, int fact_index, bool is_axiom) const override;

        int get_num_operator_effects(int index, bool is_axiom) const override;
        FactPair get_operator_effect(int op_index, int eff_index, bool is_axiom) const override;

        // Axiom-Effekt‐Preconditions

        int get_num_operator_effect_conditions(int, int, bool is_axiom) const override;
        FactPair get_operator_effect_condition(int, int, int, bool is_axiom) const override;

        int get_operator_cost(int index, bool is_axiom) const override;

    private:
        std::vector<TseitinAxiom> axioms;
        int parent_vars;
        int aux_vars;
    };

} // namespace tasks
