/*
 *  TseitinTransformer
 *
 *  Zerlegt jedes Domain-Axiom in Axiome mit höchstens zwei Preconditionen und
 *  erzeugt dafür Hilfsvariablen.  Die Klasse liefert
 *    . die vollständige Liste der transformierten Axiome
 *    . die Anzahl neu erzeugter Hilfsvariablen
 */

#include "tseitin_transformer.h"
#include <algorithm>
#include <cassert>

using namespace tasks;

/*
 *  Konstruktor: initialisiert Zähler für Hilfsvariablen und Debug-Log.
 */
TseitinTransformer::TseitinTransformer(int start_var_id)
    : next_aux_var_id(start_var_id),
      dbg("tseitin_debug.log", std::ios::out)
{
}

/*
 * make_aux: erzeugt eine neue Hilfsvariable (ID = next_aux_var_id++).
 */
FactPair TseitinTransformer::make_aux()
{
    FactPair aux(next_aux_var_id++, next_aux_value);
    dbg << "  [DEBUG] Created auxiliary var " << aux << "\n";
    return aux;
}

/*
 * encode_recursive: teilt ein Literalliste von rechts, bis <= 2 übrig sind.
 * Liefert das Kopf-Literal (erstes Element) der gekürzten Liste.
 */
FactPair TseitinTransformer::encode_recursive(std::vector<FactPair> &lits, int &layer)
{
    assert(!lits.empty());

    /* Abbruch: Liste ist bereits klein genug */
    if (lits.size() <= 2)
    {
        dbg << "  [DEBUG] encode_recursive: reached <=2 literals, stop\n";
        return lits[0];
    }

    /* Letztes Paar (a,b) auswählen */
    FactPair a = lits[lits.size() - 2];
    FactPair b = lits[lits.size() - 1];

    std::pair<FactPair, FactPair> key =
        (a.var < b.var) ? std::pair<FactPair, FactPair>(a, b)
                        : std::pair<FactPair, FactPair>(b, a);

    /* Prüfen, ob es schon eine Hilfsvariable für (a,b) gibt */
    FactPair aux(-1, -1); // default
    std::unordered_map<std::pair<FactPair, FactPair>, FactPair, FactPairHash>::iterator it =
        comb_cache.find(key);

    if (it != comb_cache.end())
    {
        aux = it->second;
        dbg << "  [DEBUG] Reusing aux " << aux << " for (" << a << "," << b << ")\n";
    }
    else
    {
        aux = make_aux();
        comb_cache.emplace(key, aux);
        axioms.push_back({{a, b}, aux, layer});
        dbg << "  [DEBUG] New aux " << aux << " for (" << a << "," << b << ")\n";
    }

    /* (a,b) durch aux ersetzen */
    lits[lits.size() - 2] = aux;
    lits.pop_back();

    dbg << "    [DEBUG] Remaining lits:";
    for (const FactPair &c : lits)
        dbg << " " << c;
    dbg << "\n";

    return encode_recursive(lits, layer);
}

/*
 * encode_axiom: zerlegt ein einzelnes Axiom (conds → eff) in Hilfsaxiome
 * und legt das finale Axiom (≤2 Prä­missen) in axioms ab.
 */
void TseitinTransformer::encode_axiom(const std::vector<FactPair> &conds,
                                      const FactPair &eff, int &layer)
{
    std::vector<FactPair> tmp = conds;

    std::sort(tmp.begin(),
              tmp.end(),
              [](const FactPair &x, const FactPair &y)
              {
                  return std::tie(x.var, x.value) < std::tie(y.var, y.value);
              });

    dbg << "[ENCODE AXIOM] pre=[";
    for (const FactPair &c : tmp)
        dbg << c << " ";
    dbg << "] -> " << eff << "\n";

    /* Hilfsaxiome erzeugen */
    encode_recursive(tmp, layer);

    /* finale Regel abspeichern (tmp hat 1 oder 2 Literale) */
    axioms.push_back({tmp, eff, layer});
    dbg << "  [DEBUG] Final axiom:";
    for (const FactPair &c : tmp)
        dbg << " " << c;
    dbg << " -> " << eff << "\n";
}

/*
 * transform: Hauptfunktion.
 *  1) Debug-Ausgabe der Originalaxiome
 *  2) Zerlegung mit encode_axiom
 *  3) Rückgabe der neuen Axiome + Anzahl Hilfsvariablen
 */
TseitinTransformer::Result TseitinTransformer::transform(TaskProxy &proxy)
{
    /* ---------- (1) Originalaxiome loggen ---------- */
    const AxiomsProxy &orig = proxy.get_axioms();
    dbg << "[ORIG AXIOMS] " << orig.size() << "\n";
    dbg << "[ORIG Variables] " << proxy.get_variables().size() << "\n";
    for (const OperatorProxy &ax : orig)
    {
        dbg << ax.get_name() << " pre=[";
        for (const FactProxy &c : ax.get_effects()[0].get_conditions())
            dbg << c.get_pair() << " ";
        dbg << "] -> " << ax.get_effects()[0].get_fact().get_pair() << "\n";
    }

    /* ---------- (2) Zerlegung durchführen ---------- */
    axioms.clear();
    comb_cache.clear();

    for (const OperatorProxy &ax : orig)
    {
        std::vector<FactPair> conds;
        for (const FactProxy &c : ax.get_effects()[0].get_conditions())
            conds.emplace_back(c.get_variable().get_id(), c.get_value());

        FactPair eff = ax.get_effects()[0].get_fact().get_pair();
        VariableProxy varProxy = proxy.get_variables()[eff.var];
        int layer = varProxy.get_axiom_layer();

        (conds.size() <= 2) ? axioms.push_back({conds, eff, layer})
                            : encode_axiom(conds, eff, layer);
    }

    /* ---------- (3) Ergebnis erzeugen ---------- */
    int num_aux_vars = next_aux_var_id - proxy.get_variables().size();
    dbg << "[NEW AXIOMS] " << axioms.size()
        << "  aux_vars=" << num_aux_vars << "\n";

    return {std::move(axioms), num_aux_vars};
}
