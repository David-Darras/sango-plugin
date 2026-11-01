# Tutorial 7: Change the wild Pokémon

In this tutorial, you choose the wild Pokémon of Route 101.
In the tall grass of Route 101, the player meets Pikachu, Eevee or Riolu.

**Time:** 10 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

Each map has an encounter table for each method: the tall grass, the water,
the fishing rods, the hordes... You give a new table with
`overworld::WildEncounter::SetTable()`. When the game reads the table of the
map, the library writes your Pokémon in it.

The game then makes the moves, the ability and the gender of your species.
The DexNav and the Pokédex show your Pokémon. The files of the game do not
change.

## Step 1: Write the table

Make the file `myhack/src/my_encounters.cc`:

```cpp
/**
 * @file my_encounters.cc
 * @brief The wild Pokémon of my ROM hack.
 */

#include "sango.h"

namespace myhack {
namespace {

// The wild Pokémon of the tall grass of Route 101: the species, the minimum
// level and the maximum level.
const overworld::EncounterSlot kRoute101Grass[] = {
    {SpeciesId::kPikachu, 3, 5},
    {SpeciesId::kEevee, 3, 5},
    {SpeciesId::kRiolu, 4, 6},
};

} // namespace

void InstallEncounters() {
  overworld::WildEncounter::SetTable(MapId::kRoute101, EncounterMethod::kWalk,
                                     kRoute101Grass);
}

} // namespace myhack
```

## Step 2: Install the tables

In `myhack/src/entrypoint.cc`, declare `myhack::InstallEncounters()` and call
it in `SetUpHack()`:

```cpp
namespace myhack {
void InstallEncounters();
}

void SetUpHack() {
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
  myhack::InstallEncounters();
}
```

## Step 3: Build and test

```bash
make myhack
```

Walk in the tall grass of Route 101. You meet Pikachu, Eevee or Riolu.

## The methods

| `EncounterMethod` | Where | Places in the table |
|---|---|---|
| `kWalk` | The tall grass, the caves | 12 |
| `kSurf` | The water | 5 |
| `kRockSmash` | The rocks of Rock Smash | 5 |
| `kOldRod`, `kGoodRod`, `kSuperRod` | The fishing rods | 3 each |
| `kHorde` | The hordes of five Pokémon | 15 (3 hordes of 5) |

The map must already have wild Pokémon for the method: `kSurf` on a map
without water does nothing. The log of the plugin then shows
`map N: no wild Pokemon for method M`.

## The chances

Each place of a table has its own chance in the game. The library puts your
Pokémon in the places in turn: the first Pokémon goes in the first place, the
second Pokémon in the second place... and it starts again at the start of your
list. With 3 Pokémon in the tall grass (12 places), each Pokémon gets 4 places.

To make a Pokémon more frequent, put it two times in the list.

## Forms

Give the form after the levels:

```cpp
{SpeciesId::kShellos, 10, 12, FormId::kShellosEast},
```

## Find the id of a map

- The map ids are in `lib/include/overworld/constant/map.h`.
- The overlay shows the current map: **Overworld > Location > Map ID**.

## Go further: the callbacks

For more control, `overworld::WildEncounter` also has two callbacks:

| Callback | When the game calls it | Use it to |
|---|---|---|
| `on_encounter_table` | The game reads the encounter table of a map, after `SetTable()`. | Change the table with your own code. |
| `on_wild_pokemon` | The game selected the wild Pokémon of a battle. | Change one battle: species, level, shiny, item, moves... |

```cpp
// `count` is 1 for a normal battle and 5 for a horde.
void OnWildPokemon(MapId map_id, overworld::WildPokemon* pokemons, u32 count) {
  for (u32 i = 0; i < count; i++) {
    pokemons[i].is_shiny = true;
  }
}

// In SetUpHack():
overworld::WildEncounter::GetInstance().on_wild_pokemon = OnWildPokemon;
```

In `on_wild_pokemon`, the game already made the moves, the ability and the
gender. If you change the species there, set them too.
`kaizo/src/kaizo_encounter.cc` uses the two callbacks.

## Disable the wild Pokémon

The overlay has the entry **Overworld > Wild Encounters**: Normal, Never or
Always. Your menu can use it too. Add this to `LoadHackMenu()`:

```cpp
static const c8* kModes[] = {"Normal", "Never", "Always"};
app.Add("Wild Encounters", overworld::WildEncounter::GetInstance().mode)
   .WithArray(kModes, SIZE(kModes));
```

## What you learned

- `WildEncounter::SetTable()` gives the wild Pokémon of a map and a method.
- The game makes the moves and the ability of your species.
- The callbacks change the tables and the battles with your own code.

**Next:** [Tutorial 8: Change the trainer teams](08-change-trainer-teams.md)
