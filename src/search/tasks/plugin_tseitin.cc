#include "tseitin_transformer.h"
#include "tseitin_task.h"
#include "root_task.h"
#include "../task_proxy.h"
#include "../plugins/plugin.h"

namespace tasks
{
    class TseitinFeature : public plugins::TypedFeature<AbstractTask, AbstractTask>
    {
    public:
        TseitinFeature() : TypedFeature("tseitin") {}

        std::shared_ptr<AbstractTask> create_component(const plugins::Options &) const override
        {
            auto base = g_root_task;
            TaskProxy proxy(*base);

            TseitinTransformer tr(proxy.get_variables().size());
            auto res = tr.transform(proxy);

            bool changed = !res.head_map.empty() || res.num_aux_vars > 0;
            if (!changed)
                return base; // identische Task → nichts verändern

            return std::make_shared<TseitinTask>(base,
                                                 std::move(res.axioms),
                                                 std::move(res.head_map),
                                                 res.num_aux_vars);
        }
    };
    static plugins::FeaturePlugin<TseitinFeature> _plugin;
} // namespace tasks
