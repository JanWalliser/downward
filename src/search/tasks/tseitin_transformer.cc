#include "tseitin_transformer.h"
#include <algorithm>
#include <cassert>

using namespace tasks;

// ---------- ctor ----------
TseitinTransformer::TseitinTransformer(int start_var)
    : next_aux_var_id(start_var),
      dbg("tseitin_debug.log", std::ios::out)
{
}

// ---------- aux ----------
FactPair TseitinTransformer::make_aux()
{
    return {next_aux_var_id++, next_aux_value}; // Domäne = {0,1}
}

// ---------- binäre Zerlegung ----------
FactPair TseitinTransformer::encode_recursive(std::vector<FactPair> &lits)
{
    assert(!lits.empty());
    if (lits.size() == 1)
        return lits[0];

    FactPair a = lits[0], b = lits[1];
    auto key = a.var < b.var ? std::pair{a.var, b.var} : std::pair{b.var, a.var};

    FactPair aux(-1, -1);
    if (auto it = comb_cache.find(key); it != comb_cache.end())
        aux = it->second;
    else
    {
        aux = make_aux();
        comb_cache.emplace(key, aux);
        axioms.push_back({{a, b}, aux});
    }
    // ersetze (a,b) durch aux u. rekursiv weiter
    lits.erase(lits.begin(), lits.begin() + 2);
    lits.insert(lits.begin(), aux);
    return encode_recursive(lits);
}

// ---------- top-level Axiom‐Ersetzung ----------
void TseitinTransformer::encode_axiom(const std::vector<FactPair> &conds,
                                      const FactPair &eff)
{
    std::vector<FactPair> tmp = conds;
    std::sort(tmp.begin(), tmp.end(),
              [](auto &x, auto &y)
              { return std::tie(x.var, x.value) < std::tie(y.var, y.value); });
    FactPair head = encode_recursive(tmp);
    axioms.push_back({{head}, eff});
}

// ---------- Hauptfunktion ----------
TseitinTransformer::Result TseitinTransformer::transform(TaskProxy &proxy)
{
    dbg << "[ORIG AXIOMS] " << proxy.get_axioms().size() << "\n";

    // ---------------- Axiome des Tasks übernehmen/ersetzen ------------
    for (auto ax : proxy.get_axioms())
    {
        std::vector<FactPair> conds;
        for (auto c : ax.get_preconditions())
            conds.push_back({c.get_variable().get_id(), c.get_value()});
        FactPair eff = ax.get_effects()[0].get_fact().get_pair();

        (conds.size() <= 1) ? axioms.push_back({conds, eff})
                            : encode_axiom(conds, eff);
    }

    // -------------- Operator-Effekte transformieren ------------------
    auto ops = proxy.get_operators();
    for (size_t op_i = 0; op_i < ops.size(); ++op_i)
    {
        auto op = ops[op_i];
        for (size_t eff_i = 0; eff_i < op.get_effects().size(); ++eff_i)
        {
            auto eff = op.get_effects()[eff_i];
            std::vector<FactPair> conds;
            for (auto c : eff.get_conditions())
                conds.push_back({c.get_variable().get_id(), c.get_value()});

            if (conds.size() <= 1)
                continue;                            // delegieren
            FactPair head = encode_recursive(conds); // erzeugt Axiome
            head_map.emplace(std::pair{static_cast<int>(op_i),
                                       static_cast<int>(eff_i)},
                             head);
        }
    }
    int aux_vars = next_aux_var_id - proxy.get_variables().size();
    dbg << "[NEW AXIOMS] " << axioms.size() << "  aux_vars=" << aux_vars << "\n";

    return {axioms, head_map, aux_vars};
}
