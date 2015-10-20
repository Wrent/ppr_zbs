# ZBS: zobecněná bisekční šířka

**VSTUPNÍ DATA:**

a = přirozené číslo
n = přirozené číslo představující počet uzlů grafu G, n≥5
m = přirozené číslo představující počet hran grafu G, m≥n
k = přirozené číslo řádu jednotek představující průměrný stupeň uzlu grafu G, n≥k≥3
G(V,E) = jednoduchý souvislý neorientovaný neohodnocený graf o n uzlech a m hranách
Doporučení pro algoritmus generování G:

Použijte generátor grafu s volbou typu grafu „-t AD“, který vygeneruje souvislý neorientovaný neohodnocený graf.

**ÚKOL:**

Nalezněte rozdělení množiny n uzlů grafu G do dvou disjunktních podmnožin X a Y tak, že podmnožina X obsahuje a uzlů, podmnožina Y obsahuje n-a uzlů a počet všech hran {u,v} takových, že u je z X a v je z Y, je minimální.

**VÝSTUP ALGORITMU:**

Výpis disjuktních množin uzlů X a Y a počet hran tyto množiny spojující.

**SEKVENČNÍ ALGORITMUS:**

Řešení existuje vždy. Vždy lze sestrojit zobecněný bisekční řez grafu. Sekvenční algoritmus je typu BB-DFS s hloubkou prohledávaného prostoru omezenou na |a|. Přípustný mezistav je definovaný rozdělením množiny uzlů na dvě disjunktní podmnožiny X a Y. Přípustná koncová řešení jsou všechna zkonstruovaná rozdělení množiny uzlů grafu G do množin X a Y. Cena, kterou minimalizujeme, je počet hran spojující X a Y.

**Těsná dolní** mez je rovna 1.

**Triviální horní** mez je rovna m.

**PARALELNÍ ALGORITMUS:**

Paralelní algoritmus je typu PBB-DFS-V.
