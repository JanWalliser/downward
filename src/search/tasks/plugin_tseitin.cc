#include "tseitin_transformer.h"
#include "tseitin_task.h"
#include "root_task.h"
#include "../task_proxy.h"
#include "../plugins/plugin.h"
#include <fstream>

namespace tasks
{
    class TseitinFeature : public plugins::TypedFeature<AbstractTask, AbstractTask>
    {
    public:
        TseitinFeature() : TypedFeature("tseitin") {}

        std::shared_ptr<AbstractTask> create_component(const plugins::Options &) const override
        {

            std::ofstream dbg("debug_plugin.log", std::ios::out);

            auto base = g_root_task;
            dbg << "=== BEFORE TRANSFORM ===\n";
            logTaskInfo(base, dbg);
            TaskProxy proxy(*base);
            TseitinTransformer tr(proxy.get_variables().size());
            auto res = tr.transform(proxy);

            if (res.num_aux_vars == 0)
            {
                dbg << "No aux vars generated, returning original task.\n";
                return base;
            }

            auto tseitin = std::make_shared<TseitinTask>(
                base,
                std::move(res.axioms),
                res.num_aux_vars);

            dbg << "\n=== AFTER TRANSFORM ===\n";
            logTaskInfo(tseitin, dbg);

            return tseitin;
        }

    private:
        static void logTaskInfo(const std::shared_ptr<AbstractTask> &task,
                                std::ofstream &dbg)
        {

            int nVars = task->get_num_variables();
            dbg << "numVariables: " << nVars << "\n";
            dbg << "domainSizes: [";
            for (int v = 0; v < nVars; ++v)
            {
                dbg << task->get_variable_domain_size(v);
                if (v + 1 < nVars)
                    dbg << " ";
            }
            dbg << "]\n";

            if (nVars > 0)
            {
                dbg << "axiomLayer[0]: " << task->get_variable_axiom_layer(0)
                    << ", defaultVal[0]: " << task->get_variable_default_axiom_value(0) << "\n";
                dbg << "axiomLayer[last]: " << task->get_variable_axiom_layer(nVars - 1)
                    << ", defaultVal[last]: " << task->get_variable_default_axiom_value(nVars - 1) << "\n";
            }

            auto init = task->get_initial_state_values();
            dbg << "initialStateSize: " << init.size() << "\n";

            int nOps = task->get_num_operators();
            int nAxs = task->get_num_axioms();
            int nGoals = task->get_num_goals();
            dbg << "numOperators: " << nOps << "\n";
            dbg << "numAxioms:    " << nAxs << "\n";
            dbg << "numGoals:     " << nGoals << "\n";

            if (nOps > 0)
                dbg << "opName[0]: " << task->get_operator_name(0, false) << "\n";
            if (nAxs > 0)
                dbg << "axName[0]: op=" << 0
                    << " name=" << task->get_operator_name(0, true) << "\n";

            if (nAxs > 0)
            {
                int pc = task->get_num_operator_preconditions(0, true);
                dbg << "ax0 precond num: " << pc << "\n";
                if (pc > 0)
                    dbg << "  pre0 cond: " << task->get_operator_precondition(0, 0, true) << "\n";

                int effs = task->get_num_operator_effects(0, true);
                dbg << "ax0 effect num: " << effs << "\n";
                if (effs > 0)
                    dbg << " operator eff0: " << task->get_operator_effect(0, 0, true) << "\n";
            }

            dbg << std::flush;
        }
    };

    static plugins::FeaturePlugin<TseitinFeature> _plugin;
} // namespace tasks
