

#include "tseitin_transformer.h"
#include "tseitin_task.h"
#include <algorithm>
#include <cassert>
#include <iostream>

namespace tasks
{

    TseitinTransformer::TseitinTransformer(int start_var_id)
        : next_aux_var_id(start_var_id),
          next_aux_value(0),
          debug_file("tseitin_debug.log", std::ios::out)
    {
        debug_file << "[TSEITIN] Initialized at var=" << start_var_id << "\n";
    }

    FactPair TseitinTransformer::make_aux()
    {
        FactPair aux{next_aux_var_id++, next_aux_value++};
        debug_file << "[TSEITIN] make_aux => (" << aux.var << "," << aux.value << ")\n";
        return aux;
    }

    size_t TseitinTransformer::hash_head_eff(const FactPair &h, const FactPair &e)
    {
        uint64_t a = (uint64_t(h.var) << 32) | uint32_t(h.value);
        uint64_t b = (uint64_t(e.var) << 32) | uint32_t(e.value);
        return size_t(a ^ (b << 1));
    }

    FactPair TseitinTransformer::encode_recursive(std::vector<FactPair> &lits)
    {
        assert(!lits.empty());
        if (lits.size() == 1)
        {
            return lits[0];
        }
        if (lits.size() == 2)
        {
            auto a = lits[0], b = lits[1];
            auto key = a.var < b.var
                           ? std::make_pair(a.var, b.var)
                           : std::make_pair(b.var, a.var);
            if (auto it = pair_to_fact.find(key); it != pair_to_fact.end())
                return it->second;
            auto aux = make_aux();
            pair_to_fact.emplace(key, aux);
            fact_to_pair.emplace(aux, key);

            axioms.push_back({{a, b}, aux});
            axioms.push_back({{aux}, a});
            axioms.push_back({{aux}, b});
            return aux;
        }
        //  kombiniere die ersten beiden, dann rekursiv
        std::vector<FactPair> first_two = {lits[0], lits[1]};
        auto head = encode_recursive(first_two);
        std::vector<FactPair> rest(lits.begin() + 2, lits.end());
        rest.insert(rest.begin(), head);
        return encode_recursive(rest);
    }

    void TseitinTransformer::encode_axiom(
        const std::vector<FactPair> &conds,
        const FactPair &eff)
    {
        // sortieren für double elim
        auto lits = conds;
        std::sort(lits.begin(), lits.end(),
                  [](auto &x, auto &y)
                  {
                      return std::tie(x.var, x.value) < std::tie(y.var, y.value);
                  });
        auto head = encode_recursive(lits);
        auto key = hash_head_eff(head, eff);
        if (seen_head_eff.insert(key).second)
            axioms.push_back({{head}, eff});
    }

    std::pair<
        std::vector<TseitinAxiom>,
        std::unordered_map<FactPair, std::pair<int, int>, FactPairHash>>
    TseitinTransformer::transform(TaskProxy &proxy)
    {
        debug_file << "[TSEITIN] transform(proxy) start\n";

        // bei 1 Precondition deligieren
        std::vector<TseitinAxiom> all_axioms;
        all_axioms.reserve(proxy.get_axioms().size());

        for (const auto &ax : proxy.get_axioms())
        {
            std::vector<FactPair> conds;
            conds.reserve(ax.get_preconditions().size());
            for (const auto &c : ax.get_preconditions())
                conds.emplace_back(c.get_variable().get_id(), c.get_value());
            auto eff = ax.get_effects()[0].get_fact().get_pair();

            if (conds.size() <= 1)
            {
                all_axioms.push_back({conds, eff});
            }
            else
            {
                // ab 2 Preconditions Tseitin encode aufrufen
                encode_axiom(conds, eff);
            }
        }

        all_axioms.insert(all_axioms.end(), axioms.begin(), axioms.end());

        debug_file << "[TSEITIN] transform(proxy) done, total axioms="
                   << all_axioms.size() << "\n";
        return {all_axioms, fact_to_pair};
    }

} // namespace tasks
