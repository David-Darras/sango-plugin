// The type chart of the game (Alpha Sapphire): one row for each attacking type, one column for
// each defending type. The plugin keeps a copy, so it can show and change the chart outside a battle.
const TypeMultiplier kOriginalChart[18][18] = {
    {k1, k1, k1, k1, k1, k05, k1, k0, k05, k1, k1, k1, k1, k1, k1, k1, k1, k1},
    {k2, k1, k05, k05, k1, k2, k05, k0, k2, k1, k1, k1, k1, k05, k2, k1, k2, k05},
    {k1, k2, k1, k1, k1, k05, k2, k1, k05, k1, k1, k2, k05, k1, k1, k1, k1, k1},
    {k1, k1, k1, k05, k05, k05, k1, k05, k0, k1, k1, k2, k1, k1, k1, k1, k1, k2},
    {k1, k1, k0, k2, k1, k2, k05, k1, k2, k2, k1, k05, k2, k1, k1, k1, k1, k1},
    {k1, k05, k2, k1, k05, k1, k2, k1, k05, k2, k1, k1, k1, k1, k2, k1, k1, k1},
    {k1, k05, k05, k05, k1, k1, k1, k05, k05, k05, k1, k2, k1, k2, k1, k1, k2, k05},
    {k0, k1, k1, k1, k1, k1, k1, k2, k1, k1, k1, k1, k1, k2, k1, k1, k05, k1},
    {k1, k1, k1, k1, k1, k2, k1, k1, k05, k05, k05, k1, k05, k1, k2, k1, k1, k2},
    {k1, k1, k1, k1, k1, k05, k2, k1, k2, k05, k05, k2, k1, k1, k2, k05, k1, k1},
    {k1, k1, k1, k1, k2, k2, k1, k1, k1, k2, k05, k05, k1, k1, k1, k05, k1, k1},
    {k1, k1, k05, k05, k2, k2, k05, k1, k05, k05, k2, k05, k1, k1, k1, k05, k1, k1},
    {k1, k1, k2, k1, k0, k1, k1, k1, k1, k1, k2, k05, k05, k1, k1, k05, k1, k1},
    {k1, k2, k1, k2, k1, k1, k1, k1, k05, k1, k1, k1, k1, k05, k1, k1, k0, k1},
    {k1, k1, k2, k1, k2, k1, k1, k1, k05, k05, k05, k2, k1, k1, k05, k2, k1, k1},
    {k1, k1, k1, k1, k1, k1, k1, k1, k05, k1, k1, k1, k1, k1, k1, k2, k1, k0},
    {k1, k05, k1, k1, k1, k1, k1, k2, k1, k1, k1, k1, k1, k2, k1, k1, k05, k05},
    {k1, k2, k1, k05, k1, k1, k1, k1, k05, k05, k1, k1, k1, k1, k1, k2, k2, k1},
};
