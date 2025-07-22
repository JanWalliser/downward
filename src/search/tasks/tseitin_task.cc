
#include "tseitin_task.h"
#include <cassert>

namespace tasks
{

    TseitinTask::TseitinTask(
        std::shared_ptr<AbstractTask> parent,
        const std::vector<TseitinAxiom> &axioms_,
        const std::unordered_map<FactPair, std::pair<int, int>, FactPairHash> &mapping)
        : DelegatingTask(parent),
          parent_vars(parent->get_num_variables()),
          aux_vars(static_cast<int>(mapping.size())),
          axioms(axioms_),
          fact_to_pair(mapping)
    {
        assert(parent);
    }

    // Wenn keine lits  ntstanden sind, delegiere komplett da tseitin unnötig ist
    int TseitinTask::get_num_variables() const
    {
        if (aux_vars == 0)
            return parent->get_num_variables();
        return parent_vars + aux_vars;
    }

    int TseitinTask::get_variable_domain_size(int var) const
    {
        if (aux_vars == 0)
            return parent->get_variable_domain_size(var);
        return var < parent_vars
                   ? parent->get_variable_domain_size(var)
                   : 2;
    }

    int TseitinTask::get_variable_axiom_layer(int var) const
    {
        if (aux_vars == 0)
            return parent->get_variable_axiom_layer(var);
        return var < parent_vars
                   ? parent->get_variable_axiom_layer(var)
                   : 0;
    }

    int TseitinTask::get_variable_default_axiom_value(int var) const
    {
        if (aux_vars == 0)
            return parent->get_variable_default_axiom_value(var);
        return var < parent_vars
                   ? parent->get_variable_default_axiom_value(var)
                   : 0;
    }

    std::vector<int> TseitinTask::get_initial_state_values() const
    {
        if (aux_vars == 0)
            return parent->get_initial_state_values();
        auto vals = parent->get_initial_state_values();
        vals.resize(get_num_variables(), 0);
        return vals;
    }

    void TseitinTask::convert_state_values_from_parent(std::vector<int> &values) const
    {
        if (aux_vars == 0)
        {
            return;
        }
        // hier nur Aux hinten anhängen
        values.resize(get_num_variables(), 0);
    }

    int TseitinTask::get_num_axioms() const
    {
        if (aux_vars == 0)
            return parent->get_num_axioms();
        return static_cast<int>(axioms.size());
    }

    int TseitinTask::get_num_operator_preconditions(int op_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_num_operator_preconditions(op_index, is_axiom);
        if (is_axiom)
            return static_cast<int>(axioms[op_index].conditions.size());
        return parent->get_num_operator_preconditions(op_index, false);
    }

    FactPair TseitinTask::get_operator_precondition(
        int op_index, int cond_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_operator_precondition(op_index, cond_index, is_axiom);
        if (is_axiom)
            return axioms[op_index].conditions[cond_index];
        return parent->get_operator_precondition(op_index, cond_index, false);
    }

    int TseitinTask::get_num_operator_effects(int op_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_num_operator_effects(op_index, is_axiom);
        if (is_axiom)
            return 1;
        return parent->get_num_operator_effects(op_index, false);
    }

    FactPair TseitinTask::get_operator_effect(
        int op_index, int eff_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_operator_effect(op_index, eff_index, is_axiom);
        if (is_axiom)
            return axioms[op_index].effect;
        return parent->get_operator_effect(op_index, eff_index, false);
    }

    int TseitinTask::get_num_operator_effect_conditions(
        int op_index, int eff_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_num_operator_effect_conditions(op_index, eff_index, is_axiom);
        if (is_axiom)
            return 0;
        return parent->get_num_operator_effect_conditions(op_index, eff_index, false);
    }

    FactPair TseitinTask::get_operator_effect_condition(
        int op_index, int eff_index, int cond_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_operator_effect_condition(op_index, eff_index, cond_index, is_axiom);
        // delegate wenn keine effekt conditions da sind
        return parent->get_operator_effect_condition(op_index, eff_index, cond_index, false);
    }

    std::string TseitinTask::get_operator_name(int op_index, bool is_axiom) const
    {
        if (aux_vars == 0)
            return parent->get_operator_name(op_index, is_axiom);
        if (is_axiom)
        {
            auto &e = axioms[op_index].effect;
            return "TseitinAxiom(" + std::to_string(e.var) + "," + std::to_string(e.value) + ")";
        }
        return parent->get_operator_name(op_index, false);
    }

    // Name- und Mutex-Methoden

    std::string TseitinTask::get_variable_name(int var) const
    {
        if (aux_vars == 0)
            return parent->get_variable_name(var);
        if (var < parent_vars)
            return parent->get_variable_name(var);
        // eigene Bezeichnung für Hilfs-Variable
        auto orig = fact_to_pair.at({var, 0});
        return "aux_var_" + std::to_string(var) +
               "_(" + std::to_string(orig.first) + "," + std::to_string(orig.second) + ")";
    }

    std::string TseitinTask::get_fact_name(const FactPair &fp) const
    {
        if (aux_vars == 0)
            return parent->get_fact_name(fp);
        if (fp.var < parent_vars)
            return parent->get_fact_name(fp);
        return "aux_fact_" + std::to_string(fp.var) + "_" + std::to_string(fp.value);
    }

    bool TseitinTask::are_facts_mutex(
        const FactPair &f1, const FactPair &f2) const
    {
        if (aux_vars == 0)
            return parent->are_facts_mutex(f1, f2);
        // echte Fakten wie gehabt
        if (f1.var < parent_vars && f2.var < parent_vars)
            return parent->are_facts_mutex(f1, f2);
        // Hilfs-Fakten niemals mutex
        return false;
    }

} // namespace tasks
