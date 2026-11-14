# Tutorial 14: Add a type

In this tutorial, you add a new type: **Storm**. The plugin has no new type, but there are
six example icons (Cosmic, Sound, Light, Plasma, Dream and Time):
they are in `assets/types`, and you can use them as examples. You can also use the
same function to change the name and the
icon of a type of the game.

> **No code?** In Sango Studio, the page **Type** has the button **+ New type**.
> The type starts with the value x1 everywhere in the chart. Then you change it
> like the other types: its name and its icon (Text tab), its effects (General tab).

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md) and [Tutorial 4](04-add-a-move.md).
**Game:** Alpha Sapphire only. Pokémon X has no support for new types.
**Tools:** Python 3 with the Pillow library (`pip install pillow`).

## How a new type works

The game has 18 types, with the numbers 0 (Normal) to 17 (Fairy). It uses the
number 18 for "no type": a Pokémon with one type has 18 for its second type.
A new type has a number from 19 to 31 (`TypeId::kFirstCustom` to
`TypeId::kLastCustom`). Storm is the first one, so it gets 19.

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
to a BCLIM file, and copy the file to the folder `types` of the data folder
of your product on the SD card:

```bash
python tools/png_to_bclim.py assets/types/Storm.png Storm.bclim
```

| Product | The file on the SD card |
|---|---|
| Your ROM hack `myhack` | `sdmc:/sango/myhack/types/Storm.bclim` |
| The overlay | `sdmc:/sango/types/Storm.bclim` |

See [Data folder](../reference/glossary.md). Keep the PNG file in the project.

## Step 2: Add the type

Choose a number that no other type uses. Put it in a header of your product,
for example `myhack/include/myhack/my_types.h`. Other files include it to know
the number of the type:

```cpp
#pragma once

#include "common.h"
#include "pokemon/constant/type.h"

namespace myhack {
/// The number of the type Storm.
constexpr TypeId kTypeStorm = static_cast<TypeId>(19);
} // namespace myhack
```

Then call `AddType()` in your `SetUpHack()` function. Include
`myhack/my_types.h` at the top of `entrypoint.cc`:

```cpp
#include "battle/patch/game_extension.h"
#include "myhack/my_types.h"

void SetUpHack() {
  // The icon is the file types/Storm.bclim of the data folder.
  battle::GameExtension::AddType({myhack::kTypeStorm, u"Storm", TypeId::kWater});
}
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
keep the value x1. Put these lines in `SetUpHack()`, after `AddType()`, and
include `battle/patch/type_chart.h` at the top of the file. Write
`using myhack::kTypeStorm;` at the start of `SetUpHack()`, or write
`myhack::kTypeStorm`:

```cpp

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
// 20 types: the 18 of the game, the number 18 ("no type") and Storm (19).
static const battle::TypeMultiplier kChart[20 * 20] = { /* ... */ };
battle::GameExtension::SetTypeChart(kChart, 20);
```

The cells of the types of the game replace the type chart of the game when a
battle loads. The cells of the new types stay in the plugin, and the inverse
battle changes them as it changes the other cells.

You do not have to write the numbers by hand: the page **Game Data > Type Chart**
of the menu has a button "Open The Table". It shows the whole chart on the
bottom screen, and a touch on a cell changes it (x0, x1/2, x1, x2). Use it to
test your values.

## Step 4: Give the type to a Pokémon and to a move

Use the number of the type in the data of the game, for example like this.
Include `pokemon/native/move_data.h` at the top of the file:

```cpp
// A move of the game gets the type.
pokemon::MoveData::GetInstance(MoveId::kThunderWave).type = kTypeStorm;

// A species keeps one type of the game and gets the new type
// (see Tutorial 16).
pokemon::Species(SpeciesId::kTornadus).WithTypes(TypeId::kFlying, kTypeStorm);
```

Run this code in your `SetUpHack()` function, after `AddType()`. In the menu
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

**Next:** [Tutorial 15: Add an item](15-add-an-item.md)

## Add a type with Sango Studio

Sango Studio writes the new types in `sdmc:/sango-studio/type/`, and the plugin
reads them at the start (`battle/patch/studio_types.h`): the list
`types.csv` (the names in all the languages), the icons in `icon/` and the type
chart `chart.bin`. The plugin registers the types with `AddType()` and gives
the cells of the chart to `TypeChart::Set()`. You write no code.

- The icon has the name of the type written on it, so each language has its own.
  The English icon is `icon/<id>.bclim`; the icon of another language is
  `icon/<language>/<id>.bclim` (`ja`, `ja_kanji`, `fr`, `it`, `de`, `es`, `ko`). In the
  Text tab of the studio, each language shows the icon of the language next to
  the name. A new type without an icon in a language shows its English icon. A
  type of the game without a file keeps the icon of the game in that language.
- A change of the list or of the chart shows when the game reads `reload.bin`
  again. A new icon shows after the game restarts.
- A type that you delete in the studio stays in the lists until the game
  restarts.
