#include "eager_search.h"
#include "search_common.h"

#include "../plugins/plugin.h"

using namespace std;

namespace plugin_eager_wastar {
class EagerWAstarSearchFeature
    : public plugins::TypedFeature<SearchAlgorithm, eager_search::EagerSearch> {
public:
    EagerWAstarSearchFeature() : TypedFeature("eager_wastar") {
        document_title("Eager weighted A* search");
        document_synopsis("");

        add_list_option<shared_ptr<Evaluator>>(
            "evals",
            "evaluators");
        add_list_option<shared_ptr<Evaluator>>(
            "preferred",
            "use preferred operators of these evaluators",
            "[]");
        add_option<bool>(
            "reopen_closed",
            "reopen closed nodes",
            "true");
        add_option<int>(
            "boost",
            "boost value for preferred operator open lists",
            "0");
        add_option<int>(
            "w",
            "evaluator weight",
            "1");

        eager_search::add_eager_search_options_to_feature(
            *this, "eager_wastar");

        document_note(
            "Open lists and equivalent statements using general eager search",
            "See corresponding notes for \"(Weighted) A* search (lazy)\"");
        document_note(
            "Note",
            "Eager weighted A* search uses an alternation open list "
            "while A* search uses a tie-breaking open list. Consequently, "
            "\n```\n--search eager_wastar([h()], w=1)\n```\n"
            "is **not** equivalent to\n```\n--search astar(h())\n```\n");
    }

    virtual shared_ptr<eager_search::EagerSearch>
    create_component(const plugins::Options &opts) const override {
    auto open_list_factoryy = search_common::create_greedy_open_list_factory(
        opts.get_list<shared_ptr<Evaluator>>("evals"),
        opts.get_list<shared_ptr<Evaluator>>("preferred"),
        opts.get<int>("boost"));

    auto eager_args = eager_search::get_eager_search_arguments_from_options(opts);
    auto pruning_method = std::get<0>(eager_args);
    auto lazy_evaluator = std::get<1>(eager_args);
    auto cost_type = std::get<2>(eager_args);
    auto bound = std::get<3>(eager_args);
    auto max_time = std::get<4>(eager_args);
    auto description = std::get<5>(eager_args);
    auto verbosity = std::get<6>(eager_args);

    auto transform = opts.get<shared_ptr<AbstractTask>>("transform", nullptr);

    return make_shared<eager_search::EagerSearch>(
        open_list_factoryy,
        /* reopen_closed = */ false,
        /* f_eval = */ nullptr,
        opts.get_list<shared_ptr<Evaluator>>("preferred"),
        pruning_method,
        lazy_evaluator,
        cost_type,
        bound,
        max_time,
        transform,
        description,
        verbosity
    );
    }
};

static plugins::FeaturePlugin<EagerWAstarSearchFeature> _plugin;
}
