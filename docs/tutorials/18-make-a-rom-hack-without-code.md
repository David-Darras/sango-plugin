# Tutorial 18: Make a ROM hack without code

In this tutorial, you make a ROM hack with text files only: you do not write
C++ and you do not build the plugin. You change the wild Pokémon, a trainer and
a species, and you write a script for a character.

**Time:** 15 minutes.
**You need:** the file `sango_maker-release.3gx`. It is in the release zip
of Sango Plugin, in the folder `sango_maker/`. You can also build it with
`make maker`, or get it from GitHub: see
[Build without devkitPro](../getting-started/06-build-without-devkitpro.md).

## How it works

**Sango Maker** is a plugin that reads text files on the SD card, in the
folder `sdmc:/sango/maker`:

| File | What it changes |
|---|---|
| `encounters.txt` | The wild Pokémon of the maps. |
| `trainers.txt` | The teams, the names, the AI and the items of the trainers. |
| `species.txt` | The stats, the types, the abilities, the TMs, the learnsets and the evolutions. |
| `story.txt` | The scripts: messages, choices, conditions, gifts, battles, warps. |

The menu of Sango Maker has all the editors of Sango Plugin. They write more
files in the same folder: the scripts (`scripts.txt`), the events of the maps
(`map_events.txt`), the settings of the maps (`zones.txt`), the texts of the
game (`texts.txt`) and the tiles (`maps/`). The translations go in
`translations/` (see [Tutorial 17](17-translate-your-rom-hack.md)).

Your ROM hack is `sango_maker-release.3gx` and the folder `sango/maker`. Give
the two to the players.

## Step 1: Install Sango Maker

1. Copy `sango_maker-release.3gx` to `luma/plugins/000400000011C500/` on the
   SD card. Remove the other `.3gx` files of this folder.
2. Make the folder `sango/maker` on the SD card.
3. Copy the example files of `maker/data/` of the project into this folder.

With the release zip, copy the content of the folder `sango_maker/` to the
root of the SD card: it has the plugin and the example files. Then do step 1
only (remove the other `.3gx` files).

## Step 2: Change the wild Pokémon

Open `sango/maker/encounters.txt` in a text editor (Notepad is good). Save it
in UTF-8.

```text
[Route 101, Walk]
Pikachu, 3, 5
Eevee, 3, 5
Riolu, 4, 6
```

- `[map, method]` starts the Pokémon of a map. The methods: `Walk` (the tall
  grass and the caves), `Surf`, `Rock Smash`, `Old Rod`, `Good Rod`,
  `Super Rod`, `Horde`.
- Each line: the species, the minimum level, the maximum level. A form can
  follow: `Shellos, 10, 12, Shellos East`.

## Step 3: Change a trainer

Open `sango/maker/trainers.txt`:

```text
[Route 102 Kid 1]
format = Single
name = Joey
ai = Casual, Competitive
money = 8
pokemon = Pichu, level 7, item Oran Berry, ability Static, nature Timid, moves Thunder Shock / Charm / Sweet Kiss
pokemon = Riolu, level 8, ivs 31, evs 0/252/4/0/0/252, moves Quick Attack / Endure / Counter
```

| Key | Value |
|---|---|
| `format` | `Single`, `Double`, `Triple`, `Rotation` or `Horde`. |
| `name` | The name of the trainer in the battle. |
| `ai` | `Casual`, `Competitive`, `Strategist`: one or more. |
| `items` | The items that the trainer uses (4 at most). |
| `money` | The prize money. The game multiplies it by the level of the last Pokémon. |
| `pokemon` | One Pokémon (6 at most): the species, then `level N`, `item X`, `ability X`, `nature X`, `form X`, `ivs N`, `evs HP/Atk/Def/SpA/SpD/Spe`, `moves A / B / C / D`, `shiny`. |

A trainer is a name of `lib/include/battle/constant/trainer.h` or a number:
`[42]`.

## Step 4: Change a species

Open `sango/maker/species.txt`:

```text
[Pikachu]
stats = 45, 70, 40, 60, 50, 100
types = Electric, Fairy
abilities = Static, Static, Lightning Rod
tms = 24, 25
learnset = 1 Thunder Shock, 5 Quick Attack, 20 Thunderbolt
evolutions = Level Up / 30 / Raichu, Item / Thunder Stone / Raichu
```

| Key | Value |
|---|---|
| `stats` | HP, Attack, Defense, Sp. Atk, Sp. Def, Speed. |
| `types` | One or two types. |
| `abilities` | The two abilities and the hidden ability. |
| `catch rate` | 3 (legendary) to 255 (the easiest). |
| `tms`, `tms off` | The TMs that the species learns, or does not learn. |
| `hms`, `hms off` | The same for the HMs. |
| `learnset` | `level move`, 24 moves at most. |
| `evolutions` | `method / value / species`, 8 at most. The value is a level, an item, a move, or 0. |

For a form: `[Charizard, Charizard Megax]`.

## Step 5: Write the story

Open `sango/maker/story.txt`. A script is a list of commands, one on each
line. `[33000]` starts the script 33000: the scripts have the numbers 33000 to
33999.

```text
[33000]
face player
if flag 0
  say Take good care of your Pokémon, {player}!
  stop
end
ask Hello, {player}!\nDo you want a Pokémon?
if yes
  give pokemon Pikachu 5
  set flag 0
else
  say Come back when you are ready.
end
```

Then give the script to something of a map:

- a character: **Overworld > Map Editor**, mode Events, field Script;
- a sign or a zone on the ground: **Overworld > Map Events**, field Script
  (see [Map events](../concepts/map-events.md)).

Give the number of your script, for example 33000. Save the changes to the SD
card in the page of the editor.

> **Warning:** Give only the number of a script of `story.txt`. A number that
> no script has can freeze the game when the player talks to the character.

### The commands

| Command | What it does |
|---|---|
| `face player` | The character turns to the player. Put it first in the script of a character. |
| `say TEXT` | Shows a message. `\n` is a new line. `{player}`, `{money}` and `{var:N}` show values. |
| `sign TEXT` | Shows a message in the window of the signs. |
| `ask TEXT` | Shows a message, then Yes / No. Then use `if yes` or `if no`. |
| `choose A / B / C` | Shows a list of choices (8 at most). Then use `if choice 1`, `if choice 2`... `if choice 0`: the player pressed B. |
| `if CONDITION` ... `else` ... `end` | Runs the lines when the condition is true. `else` is optional. An `if` can be in an other `if`. |
| `stop` | Ends the script. |
| `give item ITEM [count]`, `take item ITEM [count]` | Gives or takes items. |
| `give pokemon SPECIES LEVEL` | Gives a Pokémon. A full party sends it to the PC. |
| `give money N`, `take money N` | Gives or takes money. |
| `set flag N`, `clear flag N` | Changes a flag of the ROM hack (0 to 4095). |
| `set var N V`, `add var N V` | Changes a variable of the ROM hack (0 to 511). |
| `battle trainer TRAINER` | A battle against a trainer. Then use `if won` or `if lost`. After a loss, the player wakes up at the Pokémon Center and the script ends. |
| `battle wild SPECIES LEVEL` | A battle against a wild Pokémon. `if won` is also true after a catch. |
| `warp MAP, X, Z, HEIGHT[, DIRECTION]` | Sends the player to a map. The direction is `Up`, `Down`, `Left` or `Right`. |
| `heal` | Heals the Pokémon of the party. |
| `fade out [white]`, `fade in` | Makes the screens black (or white), then shows them again. |
| `music MUSIC`, `music end` | Plays the music of an event, then the music of the map again. |
| `wait FRAMES` | Waits (60 frames = 1 second). |
| `sound N` | Plays a sound effect. |
| `call SCRIPT` | Runs a script of the game. |

The conditions of `if`:

| Condition | True when |
|---|---|
| `yes`, `no` | The answer of the last `ask`. |
| `choice N` | The choice of the last `choose`. |
| `flag N`, `not flag N` | A flag of the ROM hack is set, or not set. |
| `var N = V` | A variable of the ROM hack has a value. Also `!=`, `>=`, `<=`, `>`, `<`. |
| `has ITEM [count]` | The Bag has the items. |
| `money N` | The player has N money or more. |
| `won`, `lost` | The result of the last battle. |

`#` starts a comment: do not use it in a text.

## Step 6: Test

Start the game. The changes are there at once: no build.

If a name is wrong, the top screen shows the problems a few seconds after the
start, with the file and the line: `encounters.txt 12: unknown species Pikachuu`.
Correct the file and start the game again.

## The editor on your computer

The project has an editor for these files: `tools/maker_editor/index.html`.
Double-click it: it opens in your web browser. It works without internet.

- The tabs are the four files: Encounters, Trainers, Species, Story.
- The editor checks the file while you type, with the same rules as Sango
  Maker. Click a problem to go to its line.
- The fields propose the names (species, moves, items, maps...).
- The forms on the right make a block (a table, a trainer, a species) and put
  it in the file. In the tab Story, the buttons add the commands.
- **Open a file** reads a file of your computer. **Download** saves the file:
  copy it to `sango/maker` on the SD card.

The editor keeps your texts in the browser between two visits.

## The names

The files use the names of the library, in English:

| What | Where to find the names |
|---|---|
| Species | `lib/include/pokemon/constant/species.h` |
| Moves | `lib/include/pokemon/constant/move.h` |
| Items | `lib/include/pokemon/constant/item.h` |
| Abilities | `lib/include/pokemon/constant/ability.h` |
| Maps | `lib/include/overworld/constant/map.h`, or the menu: **Overworld > Location** |
| Trainers | `lib/include/battle/constant/trainer.h` |
| Forms | `lib/include/pokemon/constant/form.h` |
| Evolution methods | `lib/include/pokemon/constant/evolution_method.h` |

Remove the `k` and add spaces: `kThunderShock` is `Thunder Shock`. The case,
the spaces and the signs do not matter: `THUNDER_SHOCK` and `thundershock` are
good too. A number is also good: `84` is Thunder Shock.

## In a ROM hack in C++

A product in C++ can read the same files. Call `core::DataFiles::Load()` in
`SetUpHack()`: it reads the files of the data folder of the product.

```cpp
#include "core/data_files.h"

void SetUpHack() {
  core::DataFiles::Load();
}
```

## What you learned

- Sango Maker reads `encounters.txt`, `trainers.txt`, `species.txt` and
  `story.txt` in `sdmc:/sango/maker`.
- A script of `story.txt` has a number from 33000 to 33999. The editors of the
  menu give it to a character or to an event.
- The editors of the menu write the other files in the same folder.
- The screen of the problems shows the mistakes, with the file and the line.

**Next:** [Tutorials](README.md)
