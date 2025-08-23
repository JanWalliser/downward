#include "tseitin_task.h"

namespace tasks
{

    TseitinTask::TseitinTask(std::shared_ptr<AbstractTask> parent,
                             std::vector<TseitinAxiom> axioms_,
                             int aux_vars_)
        : DelegatingTask(parent),
          axioms(std::move(axioms_)),
          parent_vars(parent->get_num_variables()),
          aux_vars(aux_vars_)
    {

        //            TaskProxy proxy(*this);
        //            std::ofstream dbg("tseitin_debug_result.log", std::ios::out);
        //            const AxiomsProxy &axioms = proxy.get_axioms();
        //            dbg << "[RES AXIOMS] " << axioms.size() << "\n";
        //            dbg << "[RES Variables] " << proxy.get_variables().size() << "\n";
        //            for (const OperatorProxy &ax : axioms)
        //            {
        //              dbg << ax.get_name() << " pre=[";
        //              for (const FactProxy &c : ax.get_effects()[0].get_conditions())
        //                dbg << c.get_pair() << " ";
        //              dbg << "] ";
        //              for (auto precond: ax.get_preconditions()) {
        //                dbg << precond.get_pair().var << "=" << precond.get_pair().value;
        //              }
        //              dbg << " -> " << ax.get_effects()[0].get_fact().get_pair() << "\n";
        //            }
    }

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
        for (const auto &ax : axioms)
        {
            if (ax.effect.var == var)
                return 2;
        }
        return parent->get_variable_domain_size(var);
    }

    int TseitinTask::get_variable_axiom_layer(int var) const
    {
        for (const auto &ax : axioms)
        {
            if (ax.effect.var == var)
                return ax.layer;
        }
        return parent->get_variable_axiom_layer(var);
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
        return 1;
    }

    FactPair TseitinTask::get_operator_precondition(int op_index, int fact_index, bool is_axiom) const
    {
        if (!is_axiom)
        {
            return parent->get_operator_precondition(op_index, fact_index, false);
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
