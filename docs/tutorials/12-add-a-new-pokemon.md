# Tutorial 12: Add a new Pokémon

In this tutorial, you add a new species to the game: **Sangoat**, a
Grass and Rock Pokémon. Skiddo evolves into Sangoat at level 40.
Sangoat uses the model of Gogoat.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md),
[Tutorial 16](16-change-a-species.md).
**Game:** Alpha Sapphire only. In X, the library does not add species.

## How it works

The game has 721 species. The library adds 297 species of generations VII,
VIII and IX (724 Rowlet to 1020 Sinistcha). Your product can add 64 more
species. A new species has these parts:

| Part | Where the library keeps it |
|---|---|
| **The data:** base stats, types, abilities, catch rate, TMs | The species table. `pokemon::SpeciesTable` makes a bigger table at the start. The table has a place for each new species. |
| **The name** | The library gives the name when the game asks for the name of the species. |
| **The learnset** | The library gives the moves when the game loads the learnset. |
| **The evolutions** | The library gives the evolutions when the game loads them. |
| **The 3D model** | A **carrier**: a model that already exists. When the game makes the model of the new species, the library asks for the model of the carrier. |

The species of the library use these carriers:

- the model of an existing species (for example a Galarian form uses the
  model of its normal form),
- or an extra form of Victini (494) or of Volcanion (721). The plugin reads
  the converted models of these forms from the SD card
  (`sdmc:/sango/pokemodel/`). The conversion tools are not public yet.

The species of your product use the model of an existing species, with one of
its forms.

`pokemon::NewSpecies()` adds a species. It gives a `pokemon::Species`: you
change the data with the `With` functions of
[Tutorial 16](16-change-a-species.md).

### The ids of the new species

The first call of `NewSpecies()` gives the id 1021. The second call gives
1022, and so on. `pokemon::NewSpeciesId(n)` gives the id of the call number
`n` (0 for the first call). Use it to write the id in your tables before the
game starts.

## Step 1: Add the species

Make the file `myhack/src/my_new_species.cc`:

```cpp
/**
 * @file my_new_species.cc
 * @brief The new species of my ROM hack.
 */

#include "core/problems.h"
#include "sango.h"

namespace myhack {

// The id of the new species. Sangoat is the first call of NewSpecies().
constexpr SpeciesId kSangoat = pokemon::NewSpeciesId(0);

namespace {

// The learnset: the level, then the move. 24 moves at most.
const pokemon::LevelMove kSangoatMoves[] = {
    {1, MoveId::kTackle},
    {1, MoveId::kVineWhip},
    {12, MoveId::kRockThrow},
    {20, MoveId::kRazorLeaf},
    {30, MoveId::kRockSlide},
    {40, MoveId::kHornLeech},
    {50, MoveId::kStoneEdge},
};

// Skiddo evolves into Sangoat at level 40.
const pokemon::EvolutionStep kSkiddoEvolutions[] = {
    {EvolutionMethod::kLevelUp, 40, kSangoat},
};

} // namespace

void AddNewSpecies() {
  // The name, the species that gives the first data and the learnset, then
  // the species that gives the model.
  const SpeciesId id =
      pokemon::NewSpecies(u"Sangoat", SpeciesId::kSkiddo, SpeciesId::kGogoat)
          // HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
          .WithStats(80, 95, 70, 60, 75, 85)
          .WithTypes(TypeId::kGrass, TypeId::kRock)
          .WithAbilities(AbilityId::kSapSipper, AbilityId::kSturdy,
                         AbilityId::kGrassPelt)
          .WithCatchRate(45)
          .WithTm(39)  // TM39: Rock Tomb.
          .WithLearnset(kSangoatMoves)
          .GetSpecies();
  if (id != kSangoat) {
    core::Problems::Add(u"Sangoat: the id is not correct");
  }

  // The evolution of Skiddo is a change of an existing species.
  pokemon::Species(SpeciesId::kSkiddo).WithEvolutions(kSkiddoEvolutions);
}

} // namespace myhack
```

The parts that you do not change keep the data of Skiddo. A new species has
no evolution. To give it evolutions, use `WithEvolutions()`.

The name must stay in memory. A text in quotes (`u"Sangoat"`) stays in memory.

## Step 2: Put the species in the game

The new species is a species like the others. Give it to the player, or put
it in the tall grass. Add this to `myhack/src/my_new_species.cc`, after
`AddNewSpecies()`:

```cpp
namespace myhack {
namespace {

// Sangoat in the tall grass of Route 101.
const overworld::EncounterSlot kRoute101Grass[] = {
    {kSangoat, 3, 5},
    {SpeciesId::kSkiddo, 3, 5},
};

} // namespace

void InstallNewSpeciesEncounters() {
  overworld::WildEncounter::SetTable(MapId::kRoute101, EncounterMethod::kWalk,
                                     kRoute101Grass);
}

} // namespace myhack
```

In a script, `ctx.GivePokemon(kSangoat, 10)` gives a Sangoat at level 10
([Tutorial 9](09-write-an-overworld-script.md)).

## Step 3: Call the functions at the start

In `myhack/src/entrypoint.cc`:

```cpp
namespace myhack {
void AddNewSpecies();
void InstallNewSpeciesEncounters();
}

void SetUpHack() {
  myhack::AddNewSpecies();
  myhack::InstallNewSpeciesEncounters();
}
```

Call `NewSpecies()` in `SetUpHack()`. The library makes the species table one
time, at the start. After the start, `NewSpecies()` cannot add a species.

## Step 4: Build and test

```bash
make myhack
```

1. Walk in the tall grass of Route 101. A wild Sangoat appears. It has the
   model of Gogoat and the name "Sangoat".
2. Catch it. In the summary, it has the types Grass and Rock.
3. Give a level to a Skiddo of level 39. It evolves into Sangoat.

## The functions

| Function | What it does |
|---|---|
| `pokemon::NewSpecies(name, data_from, model, model_form)` | Adds a species. `data_from` gives the first data and the learnset. `model` and `model_form` give the 3D model. Without `model`, the species uses the model of `data_from`. |
| `pokemon::NewSpeciesId(n)` | Gives the id of the call number `n` of `NewSpecies()`: 1021 + `n`. |
| `.GetSpecies()` | Gives the id of the species. `SpeciesId::kNone` when the library cannot add it. |
| The `With` functions | Change the data, the learnset and the evolutions. See [Tutorial 16](16-change-a-species.md). |

A model with a form: the model of Mega Charizard X, for example.

```cpp
pokemon::NewSpecies(u"Blazewing", SpeciesId::kCharmeleon,
                    SpeciesId::kCharizard, FormId::kCharizardMegax);
```

## Limits

- Alpha Sapphire only.
- 64 new species at most.
- No new 3D model: the species uses the model of an existing species.
  To change only the look of a Pokémon, see
  [Tutorial 10](10-replace-models.md).
- The library does not give a Pokédex entry, an icon or a cry to the new
  species. Test them in the game before you release your ROM hack.

## Problems

The screen of the problems (see [Troubleshooting](../getting-started/05-troubleshooting.md))
shows these mistakes:

- `NewSpecies: call it in SetUpHack()`: you called `NewSpecies()` after the start.
- `NewSpecies: more than 64 new species`: remove species.
- `NewSpecies: the name or the first data is not correct`: give a name, and a
  species of the game or of the library as `data_from`.
- `NewSpecies: Alpha Sapphire only`: you built the product for X.
- `Sangoat: the id is not correct`: the order of the calls of `NewSpecies()`
  is not the order of your `NewSpeciesId()` numbers.

## What you learned

- `pokemon::NewSpecies()` adds a species with a name, data and a carrier model.
- The ids of the new species start at 1021, in the order of the calls.
- The `With` functions of `pokemon::Species` change the new species too.

**Next:** [Tutorial 13: Add your own music](13-add-your-own-music.md)
