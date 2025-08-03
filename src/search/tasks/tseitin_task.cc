#include "tseitin_task.h"

namespace tasks
{

    TseitinTask::TseitinTask(std::shared_ptr<AbstractTask> parent,
                             std::vector<TseitinAxiom> axioms_,
                             int aux_vars_)
        : DelegatingTask(parent),
          axioms(std::move(axioms_)),
          parent_vars(parent->get_num_variables()),
          aux_vars(aux_vars_) {}

    int TseitinTask::get_num_variables() const
    {
        return parent_vars + aux_vars;
    }

    int TseitinTask::get_operator_cost(int index, bool is_axiom) const
    {
        if (is_axiom)
            return 0;
        return parent->get_operator_cost(index, is_axiom);
    }

    int TseitinTask::get_variable_domain_size(int var) const
    {
        return (var < parent_vars)
                   ? parent->get_variable_domain_size(var)
                   : 2;
    }

    int TseitinTask::get_variable_axiom_layer(int var) const
    {
        // Ändern max von beiden facts

        if (var < parent_vars)
            return parent->get_variable_axiom_layer(var);

        const TseitinAxiom *def = nullptr;
        for (const auto &ax : axioms)
        {
            if (ax.effect.var == var)
            {
                def = &ax;
                break;
            }
        }

        int max_layer_in_body = 0;
        for (const FactPair &cond : def->conditions)
        {
            max_layer_in_body = std::max(max_layer_in_body,
                                         get_variable_axiom_layer(cond.var));
        }
        return max_layer_in_body;
    }

    int TseitinTask::get_variable_default_axiom_value(int var) const
    {
        return (var < parent_vars)
                   ? parent->get_variable_default_axiom_value(var)
                   : 0;
    }

    std::vector<int> TseitinTask::get_initial_state_values() const
    {
        auto vals = parent->get_initial_state_values();
        vals.insert(vals.end(), aux_vars, 0);
        return vals;
    }

    void TseitinTask::convert_state_values_from_parent(std::vector<int> &values) const
    {
        values.resize(parent_vars + aux_vars, 0);
    }

    std::string TseitinTask::get_operator_name(int index, bool is_axiom) const
    {
        if (is_axiom)
            return "tseitin-axiom-" + std::to_string(index);
        return parent->get_operator_name(index, false);
    }

    int TseitinTask::get_num_operator_preconditions(int index, bool is_axiom) const
    {
        if (!is_axiom)
            return parent->get_num_operator_preconditions(index, false);
        if (index < parent_vars)
            return parent->get_num_operator_preconditions(index, true);
        return 1;
    }

        FactPair TseitinTask::get_operator_precondition(int op_index, int fact_index, bool is_axiom) const
    {
        if (!is_axiom)
        {
            return parent->get_operator_precondition(op_index, fact_index, false);
        }
        if (op_index < parent_vars)
        {
            return parent->get_operator_precondition(op_index, fact_index, true);
        }
        int var = get_operator_effect(op_index, 0, true).var;
        return FactPair(var, get_variable_default_axiom_value(var));
    }

    int TseitinTask::get_num_operator_effects(int index, bool is_axiom) const
    {
        if (is_axiom)
            return 1;
        return parent->get_num_operator_effects(index, false);
    }

    FactPair TseitinTask::get_operator_effect(int op_index, int eff_index, bool is_axiom) const
    {
        if (is_axiom)
            return axioms[op_index].effect;
        return parent->get_operator_effect(op_index, eff_index, false);
    }

    int TseitinTask::get_num_operator_effect_conditions(int op_index, int eff_index, bool is_axiom) const
    {
        if (is_axiom)
        {
            return axioms[op_index].conditions.size();
        }
        else
        {
            return parent->get_num_operator_effect_conditions(op_index, eff_index, false);
        }
    }
    FactPair TseitinTask::get_operator_effect_condition(int op_index, int eff_index, int cond_index, bool is_axiom) const
    {
        if (is_axiom)
        {
            return axioms[op_index].conditions[cond_index];
        }
        else
        {
            return parent->get_operator_effect_condition(op_index, eff_index, cond_index, false);
        }
    }

} // namespace tasks
