Sango Plugin {{VERSION}} for Pokémon Alpha Sapphire v1.4 (no ROM patch).

**New in {{VERSION}}**

- **Sango Maker is complete: a ROM hack with text files, without code.** The files are in `sango/maker/` of the SD card:
  `encounters.txt` (the wild Pokémon of each map), `trainers.txt` (the teams), `species.txt` (stats, types, abilities, learnsets, evolutions, and new species),
  `story.txt` (the scripts of the characters), `characters.txt`, `rules.txt` (the rules of the battles, the shiny rate, the starters),
  `shops.txt` (the items and the Pokémon of each shop), `items.txt` and `moves.txt` (the data of the items and of the moves).
  The menu page **Sango Maker** shows the problems of the files. **Live Reload is on by default**: the plugin reads a file again a few seconds after you save it.
  `maker_log.txt` in the same folder says which files were read, and how many problems each one has.
- **A line outside a `[section]` is now reported** instead of being ignored (for example an item in `shops.txt` before `[shop N]` or `[all shops]`).
- **The 948 trainers of the game have a name** (`TrainerId`): the map, the title and the name, for example `Rustboro City Leader Roxanne`. A rematch ends with `Rematch`.
  Use the names in `trainers.txt`, in `story.txt` and in C++. `tools/export_trainers.py` lists the trainers with their maps and writes all the teams of the game in the format of `trainers.txt`.
- **Export to the Maker format** (menu > Sango Maker): the wild Pokémon of the map, a species and the team of the player.
- **A plugin twice as small, and a bigger heap.** The `.3gx` files are half the size of the files of 7.3.0 (the same tools build both):

  | File | 7.3.0 | {{VERSION}} | Gain |
  |---|---|---|---|
  | `sango_plugin.3gx` (the overlay) | 2 258 KiB | 1 115 KiB | **-1 143 KiB (-51%)** |
  | `sango_kaizo.3gx` | 1 547 KiB | 766 KiB | **-781 KiB (-50%)** |

  The code alone is more than 50% smaller, and the memory that the plugin uses (code, data and variables) is 1.2 MB smaller for the overlay and 0.9 MB for Kaizo.
  The causes: the build removes the code that nothing uses (`-Os` and LTO), the variables that were not needed are gone, the test content is out of the release,
  the buffers come from the heap, and the plugin does not use libctrpf any more (it has its own start-up code: 186 KiB less alone, and about 700 KB of heap that libctrpf kept for itself).
- **Fixed:** the wild Pokémon of `encounters.txt` now apply when the map loads (they applied only in the DexNav before), `moves.txt` now changes the moves in the battles, the menus and the PP,
  and the name of Shiftry.
- libctrpf is not needed to build the plugin any more: only devkitPro and 3gxtool. The tutorials, the Docker image and the GitHub workflow say so.

**Install**

1. Download `Sango-Plugin-{{VERSION}}.zip` below and unzip it at the root of the SD card.
   On Azahar: `%APPDATA%/Azahar/sdmc/`.
2. Download the packs. **The `.7z` files have no folder inside**: make each folder yourself (in `sango/`), then unzip the pack in it.
   - Pokémon models (the 3D models of the Pokémon, needed for the new Pokémon): {{POKEMON_MODELS_URL}}
     → unzip in `sango/pokemodel/`
   - Overworld models (the 3D models of the characters, needed for the new characters): {{OVERWORLD_MODELS_URL}}
     → unzip in `sango/overworld_model/`
   - Optional, the music of Pokémon X and Y: {{MUSIC_URL}}
     → unzip in `sango/bgm/` (the pack has its own `list.txt`)
3. Azahar: Emulation > Configure > System > Enable 3GX Plugin Loader.
4. Start the game, and press Start in the overworld to open the menu.

Do not save the game with a new Pokémon in your party: this can corrupt your save file.

The zip has a `README.txt` with the same steps.

**For the modders:** the zip also has **Sango Maker** (`sango_maker/`), the example text files and the web editor (`maker_editor/index.html`).
Sango Maker makes a ROM hack with text files, without code. Part 2 of `README.txt` explains how to install it.
Its menu page **Sango Maker** reads the files again without a restart (also when they change), lists their problems and changes the flags and the variables.

Feedback and bug reports are welcome.
