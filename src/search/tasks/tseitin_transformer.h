#ifndef TASKS_TSEITIN_TRANSFORMER_H
#define TASKS_TSEITIN_TRANSFORMER_H

#include "../task_proxy.h"
#include <vector>
#include <unordered_map>
#include <fstream>

namespace tasks
{

    struct TseitinAxiom
    {
        std::vector<FactPair> conditions;
        FactPair effect;
    };

    struct IntPairHash
    {
        size_t operator()(const std::pair<int, int> &p) const noexcept
        {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };

    struct FactPairHash
    {
        size_t operator()(const std::pair<FactPair, FactPair> &p) const noexcept
        {
            return std::hash<int>()(p.first.var) ^ (std::hash<int>()(p.first.value) << 1) ^ (std::hash<int>()(p.second.value) << 2) ^ (std::hash<int>()(p.second.value) << 3);
        }
    };

    class TseitinTransformer
    {
        int next_aux_var_id;
        static constexpr int next_aux_value = 1;

        std::unordered_map<std::pair<FactPair, FactPair>, FactPair, FactPairHash> comb_cache;
        std::vector<TseitinAxiom> axioms;
        std::ofstream dbg;

        FactPair make_aux();
        FactPair encode_recursive(std::vector<FactPair> &lits);
        void encode_axiom(const std::vector<FactPair> &conds, const FactPair &eff);

    public:
        explicit TseitinTransformer(int start_var_id);

        struct Result
        {
            std::vector<TseitinAxiom> axioms;
            int num_aux_vars;
        };

        Result transform(TaskProxy &proxy);
    };

} // namespace tasks

#endif // TASKS_TSEITIN_TRANSFORMER_H
