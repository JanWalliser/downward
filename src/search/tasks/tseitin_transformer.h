// src/search/tasks/tseitin_transformer.h

#ifndef TASKS_TSEITIN_TRANSFORMER_H
#define TASKS_TSEITIN_TRANSFORMER_H

#include "../abstract_task.h"
#include "../task_proxy.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <fstream>

namespace tasks
{

    // Tseitin-Axiom: Bedingungen ->Effekt
    struct TseitinAxiom
    {
        std::vector<FactPair> conditions;
        FactPair effect;
    };

    // Hash für FactPair
    struct FactPairHash
    {
        size_t operator()(const FactPair &f) const noexcept
        {
            return std::hash<int>()(f.var) ^ (std::hash<int>()(f.value) << 1);
        }
    };

    struct IntPairHash
    {
        size_t operator()(const std::pair<int, int> &p) const noexcept
        {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };

    class TseitinTransformer
    {
        int next_aux_var_id;
        int next_aux_value;
        std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> pair_to_fact;
        std::unordered_map<FactPair, std::pair<int, int>, FactPairHash> fact_to_pair;
        std::unordered_set<size_t> seen_head_eff;
        std::vector<TseitinAxiom> axioms;
        std::ofstream debug_file;

        FactPair make_aux();
        FactPair encode_recursive(std::vector<FactPair> &lits);
        static size_t hash_head_eff(const FactPair &h, const FactPair &e);

    public:
        explicit TseitinTransformer(int start_var_id);

        void encode_axiom(const std::vector<FactPair> &conds, const FactPair &eff);

        std::pair<
            std::vector<TseitinAxiom>,
            std::unordered_map<FactPair, std::pair<int, int>, FactPairHash>>
        transform(TaskProxy &proxy);

        const std::unordered_map<FactPair, std::pair<int, int>, FactPairHash> &
        get_fact_mapping() const { return fact_to_pair; }
    };

} // namespace tasks

#endif // TASKS_TSEITIN_TRANSFORMER_H
