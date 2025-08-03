#include "tseitin_task.h"
using namespace tasks;

/* ctor */
TseitinTask::TseitinTask(std::shared_ptr<AbstractTask> parent,
                         std::vector<TseitinAxiom> axioms_,
                         std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> heads_,
                         int aux_vars_)
    : DelegatingTask(parent), axioms(std::move(axioms_)), heads(std::move(heads_)), parent_vars(parent->get_num_variables()), aux_vars(aux_vars_)
{
}

/* ---------- Variablen ---------- */
int TseitinTask::get_num_variables() const
{
    return parent_vars + aux_vars; // identisch → aux_vars==0
}
int TseitinTask::get_variable_domain_size(int v) const
{
    return (v < parent_vars) ? parent->get_variable_domain_size(v) : 2;
}
int TseitinTask::get_variable_axiom_layer(int v) const
{
    return (v < parent_vars) ? parent->get_variable_axiom_layer(v) : 0;
}
int TseitinTask::get_variable_default_axiom_value(int v) const
{
    return (v < parent_vars) ? parent->get_variable_default_axiom_value(v) : 0;
}
std::vector<int> TseitinTask::get_initial_state_values() const
{
    auto s = parent->get_initial_state_values();
    s.insert(s.end(), aux_vars, 0);
    return s;
}
void TseitinTask::convert_state_values_from_parent(std::vector<int> &values) const
{
    /* values enthält bereits alle Werte des Parent-Tasks.
       Wir müssen nur Platz für die Hilfs-Variablen schaffen
       (Default = 0). */
    values.resize(parent_vars + aux_vars, 0);
}

/* ---------- Axiome ---------- */
int TseitinTask::get_num_axioms() const
{
    return static_cast<int>(axioms.size());
}
int TseitinTask::get_num_operator_preconditions(int op, bool ax) const
{
    return ax ? static_cast<int>(axioms[op].conditions.size())
              : parent->get_num_operator_preconditions(op, false);
}
FactPair TseitinTask::get_operator_precondition(int op, int idx, bool ax) const
{
    return ax ? axioms[op].conditions[idx]
              : parent->get_operator_precondition(op, idx, false);
}

/* ---------- Effekt-Prä-Conditions ---------- */
int TseitinTask::get_num_operator_effect_conditions(int op, int eff, bool ax) const
{
    if (ax)
        return 0;
    return heads.count({op, eff}) ? 1
                                  : parent->get_num_operator_effect_conditions(op, eff, false);
}
FactPair TseitinTask::get_operator_effect_condition(int op, int eff, int idx, bool ax) const
{
    if (ax)
        return FactPair::no_fact;
    auto it = heads.find({op, eff});
    return (it != heads.end()) ? it->second
                               : parent->get_operator_effect_condition(op, eff, idx, false);
}

/* ---------- Debug-Name ---------- */
std::string TseitinTask::get_operator_name(int op, bool ax) const
{
    return ax ? "tseitin-axiom-" + std::to_string(op)
              : parent->get_operator_name(op, false);
}
