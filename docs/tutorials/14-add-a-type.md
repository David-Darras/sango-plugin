# Tutorial 14: Add a type

In this tutorial, you add a seventh new type: **Storm**. The overlay already has
six new types (Cosmic, Sound, Light, Plasma, Dream and Time): they are in
`overlay/src/custom_types.cc`, and you can use them as examples. You can also
use the same function to change the name and the icon of a type of the game.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md) and [Tutorial 4](04-add-a-move.md).
**Game:** Alpha Sapphire only. Pokémon X has no support for new types.
**Tools:** Python 3 with the Pillow library (`pip install pillow`).

## How a new type works

The game has 18 types, with the numbers 0 (Normal) to 17 (Fairy). It uses the
number 18 for "no type": a Pokémon with one type has 18 for its second type.
A new type has a number from 19 to 31 (`TypeId::kFirstCustom` to
`TypeId::kLastCustom`). The overlay uses the numbers 19 to 24, so Storm gets 25.

`battle::GameExtension` makes the game accept the number:

- It gives the name of the type to the game, in the battle messages and in the
  menus.
- It gives the icon to the layouts of the game: the choice of the move, the
  summary, the party, the bag and the PC box.
- It reads the effect of the type from a type chart that you give.

The icon is a file on the SD card: `sdmc:/sango/types/<name>.bclim`, with the
name of the type (for example `Cosmic.bclim` for the type "Cosmic"). You do not
give the path. The buttons of the moves in a battle also have a color for each
type. A new type uses the color of a type of the game that you choose.

## Step 1: Make the icon

A type icon is an image of 50 × 20 pixels. Three tools are in the `tools`
folder:

| Tool | What it does |
|---|---|
| `make_type_icon.py` | Draws an icon in the style of the game: a pill with the name of the type. |
| `png_to_bclim.py` | Changes a PNG of 50 × 20 pixels to the format that the game reads (a BCLIM file). |
| `release.py` | Makes the BCLIM files of `assets/types` again when it builds the release zip. |

Draw the icon. The arguments are the text (the letters A to Z), the PNG file,
and the colors of the left and of the right of the pill (hexadecimal RGB):

```bash
python tools/make_type_icon.py STORM assets/types/Storm.png 1f4e8c 7fc8f0
```

You can also draw the PNG with any program: keep the size 50 × 20 pixels.

The file has the name of the type, with a capital: `Storm.png`. Then change it
to a BCLIM file, and copy the file to the SD card (for the emulator, the folder
`sdmc/sango/types`):

```bash
python tools/png_to_bclim.py assets/types/Storm.png Storm.bclim
```

The file goes to `sdmc:/sango/types/Storm.bclim`. The release script makes the
BCLIM files of all the PNG files of `assets/types` by itself, and it puts them
in the zip: keep the PNG file in your project.

## Step 2: Add the type

Choose a number that no other type uses. In the overlay, add it to
`overlay/include/custom_battle.h`:

```cpp
constexpr TypeId kTypeStorm = static_cast<TypeId>(25);
```

Then call `AddType()` in `RegisterCustomTypes()` (`overlay/src/custom_types.cc`).
In your own product, call it in your `Initialize()` function:

```cpp
#include "battle/patch/game_extension.h"

// The icon is the file sdmc:/sango/types/Storm.bclim.
battle::GameExtension::AddType({kTypeStorm, u"Storm", TypeId::kWater});
```

The fields are:

| Field | Meaning |
|---|---|
| `id` | The number of the type. |
| `name` | The name that the game shows. It is also the name of the icon file: `sdmc:/sango/types/<name>.bclim`. Without the file, a new type shows the icon of Fairy, and a type of the game keeps its icon. `nullptr` keeps the name and the icon of the type of the game. |
| `colour_type` | The type of the game whose color the buttons of the moves use. |

To change a type of the game, use its number (0 to 17): the name and the icon
change, and the type keeps its effects.

## Step 3: Give the type chart

The type chart tells how much damage a type does to another type. There are
two ways to give it.

**One cell at a time.** `TypeChart::Set()` changes one cell. The other cells
keep the value x1:

```cpp
#include "battle/patch/type_chart.h"

// Storm hits Flying and Fire well, and Grass badly.
battle::TypeChart::Set(kTypeStorm, TypeId::kFlying, battle::TypeMultiplier::k2);
battle::TypeChart::Set(kTypeStorm, TypeId::kFire, battle::TypeMultiplier::k2);
battle::TypeChart::Set(kTypeStorm, TypeId::kGrass, battle::TypeMultiplier::k05);
// Electric and Ice hit Storm well.
battle::TypeChart::Set(TypeId::kElectric, kTypeStorm, battle::TypeMultiplier::k2);
battle::TypeChart::Set(TypeId::kIce, kTypeStorm, battle::TypeMultiplier::k2);
```

**A whole table.** `GameExtension::SetTypeChart()` takes a table with one row
for each attacking type and one column for each defending type. The numbers
of the table are the 18 types of the game, then the new types in the order of
their numbers (19 is the number 18 of the table):

```cpp
// 26 types: the 18 of the game and the 8 new types (19 to 26).
static const battle::TypeMultiplier kChart[26 * 26] = { /* ... */ };
battle::GameExtension::SetTypeChart(kChart, 26);
```

The cells of the types of the game replace the type chart of the game when a
battle loads. The cells of the new types stay in the plugin, and the inverse
battle changes them as it changes the other cells.

You do not have to write the numbers by hand: the page **Battle > Type Chart**
of the menu has a button "Open The Table". It shows the whole chart on the
bottom screen, and a touch on a cell changes it (x0, x1/2, x1, x2). Use it to
test your values.

## Step 4: Give the type to a Pokémon and to a move

Use the number of the type in the data of the game. The overlay does it in
`overlay/src/type_team.cc`:

```cpp
#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"

// A move of the game gets the type.
pokemon::MoveData::GetInstance(MoveId::kThunderWave).type = kTypeStorm;

// A species keeps one type of the game and gets the new type.
auto& data = pokemon::SpeciesData::GetInstance(SpeciesId::kTornadus,
                                               FormId::kNormal);
data.type[0] = TypeId::kFlying;
data.type[1] = kTypeStorm;
```

Run this code in your `Initialize()` function, after `AddType()`. In the menu
of the plugin, the lists of types show the new types after "None" (the page of
the species data and the page of the moves).

## Check

1. Build with `make myhack` and start a battle with a move or a Pokémon of the
   new type.
2. The choice of the move shows the name and the icon of the type. If it shows
   the icon of Fairy, the file is missing or it is not a BCLIM file of
   50 × 20 pixels: look at the log of the plugin.
3. The messages "It's super effective!" use your type chart.

## What the plugin does not change

- The Pokédex and the search of the PC box have their own list of 18 icons.
  They hide the icon of a new type, and they do not use it as a filter.
- The effects of the battle that depend on a type of the game (for example the
  animation of a move) keep the choice of the game.
