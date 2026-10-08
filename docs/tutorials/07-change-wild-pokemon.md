# Tutorial 7: Change the wild Pokémon

In this tutorial, you choose the wild Pokémon of Route 101.
On Route 101, the player meets Pikachu, Eevee or Riolu at level 5.

**Time:** 15 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

`overworld::WildEncounter` has two callbacks:

| Callback | When the game calls it | Use it to |
|---|---|---|
| `on_wild_pokemon` | The game selected the wild Pokémon of a battle. | Change the Pokémon of the battle: species, level, form... |
| `on_encounter_table` | The game reads the encounter table of a map. | Change the table: the DexNav and the Pokédex show your Pokémon. |

The callbacks change the data in memory. The files of the game do not change.

## Step 1: Write the encounters

Make the file `myhack/src/my_encounters.cc`:

```cpp
/**
 * @file my_encounters.cc
 * @brief The wild Pokémon of my ROM hack.
 */

#include "common.h"
#include "core/utils.h"
#include "overworld/constant/map.h"
#include "overworld/native/wild_pokemon.h"
#include "overworld/patch/wild_encounter.h"

namespace myhack {
namespace {

// The wild Pokemon of one map.
struct MapEncounters {
  MapId map_id;
  const SpeciesId* species;
  u32 species_count;
  u8 level;
};

const SpeciesId kRoute101[] = {
    SpeciesId::kPikachu,
    SpeciesId::kEevee,
    SpeciesId::kRiolu,
};

const MapEncounters kEncounters[] = {
    {MapId::kRoute101, kRoute101, SIZE(kRoute101), 5},
};

// The game calls this function after it selects the wild Pokemon of a
// battle. `count` is 1 for a normal battle and 5 for a horde.
void OnWildPokemon(MapId map_id, overworld::WildPokemon* pokemons,
                   u32 count) {
  for (const MapEncounters& map : kEncounters) {
    if (map.map_id != map_id) continue;

    for (u32 i = 0; i < count; i++) {
      // GetRandomValue(n) returns a number from 0 to n - 1.
      const u32 index = core::Utils::GetRandomValue(map.species_count);
      pokemons[i].species = map.species[index];
      pokemons[i].form = FormId::kNormal;
      pokemons[i].level = map.level;
    }
    return;
  }
}

} // namespace

void InstallEncounters() {
  overworld::WildEncounter::GetInstance().on_wild_pokemon = OnWildPokemon;
}

} // namespace myhack
```

## Step 2: Install the callback

In `myhack/src/entrypoint.cc`, declare `myhack::InstallEncounters()` and call
it in `InstallCallbacks()`:

```cpp
namespace myhack {
void InstallEncounters();
}

void InstallCallbacks() {
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
  myhack::InstallEncounters();
}
```

## Step 3: Build and test

```bash
make myhack
```

Walk in the tall grass of Route 101. You meet Pikachu, Eevee or Riolu at level 5.

## The data of a wild Pokémon

`overworld::WildPokemon` contains:

| Member | Meaning |
|---|---|
| `species` | The species. |
| `form` | The form. Use `FormId::kNormal` for the normal form. |
| `level` | The level. |
| `is_shiny` | `true` for a shiny Pokémon. |
| `item` | The held item. |
| `ability` | The ability. |
| `moves[4]` | The moves. |
| `gender` | The gender. |

The game fills all the members before your callback. If you change the
species, the other members (moves, ability, gender) still come from the first
species. Set them too when you need exact values.

## Find the id of a map

- The map ids are in `lib/include/overworld/constant/map.h`.
- The overlay shows the id of the current map: **Overworld > Map Id**.

## Change the encounter table

`on_encounter_table` changes the table of the map. The DexNav then shows your
Pokémon. Look at `kaizo/src/kaizo_encounter.cc`: the function
`PatchEncounterTable` replaces each species of the table.

## Disable the wild Pokémon

The overlay has the entry **Repel**. It uses the cheat code
`CheatCodeId::kNoEncounter`. Your menu can use it too:

```cpp
app.Add("Repel", CheatCodeId::kNoEncounter);
```

## What you learned

- `on_wild_pokemon` changes the Pokémon of a wild battle.
- `on_encounter_table` changes the encounter table of a map.
- The map ids are in `overworld/constant/map.h`.

**Next:** [Tutorial 8: Change the trainer teams](08-change-trainer-teams.md)
