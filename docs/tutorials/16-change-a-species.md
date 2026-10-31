# Tutorial 16: Change a species

In this tutorial, you change Pikachu: its base stats, its types, its
abilities, a TM, its learnset and its evolutions.

**Time:** 15 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

`pokemon::Species` changes the data of one species (or of one form). Each
`With` function changes one part. The parts that you do not change keep the
data of the game.

Call it in `SetUpHack()`. The new species and the new forms of the library
exist only after the start: the library keeps your changes and applies them
when the data is ready.

## Step 1: Write the changes

Make the file `myhack/src/my_species.cc`:

```cpp
/**
 * @file my_species.cc
 * @brief The species changes of my ROM hack.
 */

#include "sango.h"

namespace myhack {
namespace {

// The learnset: the level, then the move. 24 moves at most.
const pokemon::LevelMove kPikachuMoves[] = {
    {1, MoveId::kThunderShock},
    {1, MoveId::kGrowl},
    {5, MoveId::kQuickAttack},
    {13, MoveId::kElectroBall},
    {20, MoveId::kThunderbolt},
    {30, MoveId::kDazzlingGleam},
};

// The evolutions: the method, its value, then the new species.
const pokemon::EvolutionStep kPikachuEvolutions[] = {
    {EvolutionMethod::kLevelUp, 30, SpeciesId::kRaichu},
    {EvolutionMethod::kItem, ItemId::kThunderStone, SpeciesId::kRaichu},
};

} // namespace

void ChangeSpecies() {
  pokemon::Species(SpeciesId::kPikachu)
      // HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
      .WithStats(45, 70, 40, 60, 50, 100)
      .WithTypes(TypeId::kElectric, TypeId::kFairy)
      // The two abilities, then the hidden ability.
      .WithAbilities(AbilityId::kStatic, AbilityId::kStatic,
                     AbilityId::kLightningRod)
      .WithTm(24)  // TM24: Thunderbolt.
      .WithLearnset(kPikachuMoves)
      .WithEvolutions(kPikachuEvolutions);
}

} // namespace myhack
```

## Step 2: Call it at the start

In `myhack/src/entrypoint.cc`:

```cpp
namespace myhack {
void ChangeSpecies();
}

void SetUpHack() {
  myhack::ChangeSpecies();
}
```

## Step 3: Build and test

```bash
make myhack
```

1. Look at a Pikachu in the summary: it has the types Electric and Fairy.
2. Open **Pokémon > Game Data > Species** in the overlay to see the stats.
3. Give Pikachu a level: it learns the moves of your learnset, and it evolves
   at level 30.

## The functions

| Function | What it changes |
|---|---|
| `WithStats(hp, atk, def, spa, spd, spe)` | The base stats, in the order of the summary. |
| `WithTypes(first, second)` | The two types. |
| `WithType(type)` | One type. |
| `WithAbilities(first, second, hidden)` | The two abilities and the hidden ability. |
| `WithCatchRate(rate)` | The catch rate: 3 for a legendary Pokémon, 255 for the easiest. |
| `WithTm(number, can_learn)` | A TM (1 to 100): `true` to learn it, `false` to stop it. |
| `WithHm(number, can_learn)` | An HM (1 to 7). |
| `WithLearnset(moves)` | The moves that the species learns when its level increases. 24 moves at most. |
| `WithEvolutions(steps)` | The evolutions. 8 at most. They are for all the forms of the species. |

## A form

Give the form after the species:

```cpp
pokemon::Species(SpeciesId::kCharizard, FormId::kCharizardMegax)
    .WithStats(78, 140, 111, 120, 85, 100);
```

## The values of an evolution

The value depends on the method:

| Method | Value | Example |
|---|---|---|
| `kLevelUp` | The level | `{EvolutionMethod::kLevelUp, 16, SpeciesId::kIvysaur}` |
| `kItem`, `kTradeItem`, `kHoldItemDay`... | The item | `{EvolutionMethod::kItem, ItemId::kFireStone, SpeciesId::kArcanine}` |
| `kKnowMove` | The move | `{EvolutionMethod::kKnowMove, MoveId::kRollout, SpeciesId::kLickilicky}` |
| `kFriendship`, `kTrade`, `kDay`... | No value: 0 | `{EvolutionMethod::kFriendship, 0, SpeciesId::kCrobat}` |

The methods are in `lib/include/pokemon/constant/evolution_method.h`.

## Problems

The screen of the problems (see [Troubleshooting](../getting-started/05-troubleshooting.md))
shows these mistakes:

- `species N: TM N does not exist`: use a TM from 1 to 100, an HM from 1 to 7.
- `species N: more than 24 learnset moves`: the library keeps the first 24 moves.
- `species N: more than 8 evolutions`: the library keeps the first 8 evolutions.

## What you learned

- `pokemon::Species` changes the data of a species or of a form.
- `LevelMove` describes a move of a learnset, `EvolutionStep` an evolution.
- The tables of moves and evolutions must stay in memory: use global `const` arrays.

**Next:** [Tutorials](README.md)
