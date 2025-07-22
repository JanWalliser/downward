

#include "tseitin_task.h"
#include "tseitin_transformer.h"
#include "root_task.h"
#include "../plugins/plugin.h"
#include "../task_proxy.h"
#include <utility>

using namespace std;

namespace tasks
{

    class TseitinTaskFeature : public plugins::TypedFeature<AbstractTask, TseitinTask>
    {
    public:
        TseitinTaskFeature() : TypedFeature("tseitin")
        {
            document_title("Tseitin transformation for axioms");
            document_synopsis("Sortes all preconditions and Recursively combines them into Tseitin literals.");
        }

        shared_ptr<TseitinTask> create_component(const plugins::Options &) const override
        {
            auto parent = g_root_task;
            TaskProxy proxy(*parent);
            TseitinTransformer transformer(parent->get_num_variables());

            // transform(proxy) gibt zurück  pair<axioms,mapping>
            auto result = transformer.transform(proxy);
            auto &axioms = result.first;
            auto &mapping = result.second;
            return make_shared<TseitinTask>(parent, axioms, mapping);
        }
    };

    static plugins::FeaturePlugin<TseitinTaskFeature> _plugin;

} // namespace tasks
