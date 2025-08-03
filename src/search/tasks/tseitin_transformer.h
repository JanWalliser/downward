#ifndef TASKS_TSEITIN_TRANSFORMER_H
#define TASKS_TSEITIN_TRANSFORMER_H

#include "../task_proxy.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <fstream>

namespace tasks
{

    // ---------------------------
    //  Datenstrukturen
    // ---------------------------
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

    // ---------------------------
    //  TseitinTransformer
    // ---------------------------
    class TseitinTransformer
    {
        int next_aux_var_id;    // nächste freie Variable-ID
        int next_aux_value = 0; // stets 0/1-Domäne → Wert immer 0/1

        //  Memoisierung für (a,b) → aux-Literal
        std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> comb_cache;
        //  Hilfsaxiome + übernommene Originalaxiome
        std::vector<TseitinAxiom> axioms;
        //  Map (opIdx,effIdx) → headLiteral  (für Task)
        std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> head_map;

        std::ofstream dbg;

        // Hilfsmethoden
        FactPair make_aux();
        FactPair encode_recursive(std::vector<FactPair> &lits);
        void encode_axiom(const std::vector<FactPair> &conds, const FactPair &eff);

    public:
        explicit TseitinTransformer(int start_var_id);

        struct Result
        {
            std::vector<TseitinAxiom> axioms;
            std::unordered_map<std::pair<int, int>, FactPair, IntPairHash> head_map;
            int num_aux_vars; // wie viele Hilfs-Variablen erzeugt
        };
        Result transform(TaskProxy &proxy);
    };

} // namespace tasks
#endif
