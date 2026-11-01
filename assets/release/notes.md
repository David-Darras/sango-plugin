Sango Plugin {{VERSION}} for Pokémon Alpha Sapphire v1.4 (no ROM patch).

**New in {{VERSION}}**

- **New Pokémon types** (`battle::GameExtension::AddType()`): add a type, or change the name and the icon of a type of the game.
  Each type has its own icon, its own row and column in the type chart, and a table editor in the menu (Battle > Type Chart).
  The overlay has six examples: **Cosmic**, **Sound**, **Light**, **Plasma**, **Dream** and **Time**. Their icons are in `sango/types/` of the zip
  (the release script makes them from the PNG files of `assets/types`).
- **Four new moves of these types**, each with its own effect in battle, its own reactions and its own animation:
  Prism Lance (Light), Sonic Requiem (Sound), Ion Storm (Plasma) and Dreamfall (Dream).
- **New items and changed items** (`battle::ItemExtension`): add an item (ids 800 to 1023) or change an item of the game (ids 1 to 775).
  An item has a name, a description, an icon (a file `sango/items/<name>.bclim`, made from a PNG), data (price, Fling...) and reactions in battle.
  The names show in the party, the summary, the Bag, the PC boxes and the overlay. The icons of the examples are in `sango/items/` of the zip
  (the release script makes them from the PNG files of `assets/items`).
- **Items that the player uses in the Bag**: the player chooses "Use" and a Pokémon, and a function of the product runs on it.
  The overlay has examples: **Shiny Switch** (shiny on and off), **Ability Switch** (goes to the next ability), **Evolution Prism** (makes a Pokémon evolve),
  **Devolution Stone** (goes back to the species before, with the evolution animation) and **Knife** (a Pokémon keeps 1 HP).
- **Medicine effects with names** (`battle::ItemMedicine`): restore HP, revive, cure the status conditions, raise the level, restore or raise the PP,
  change the EVs and the friendship. The game does the work and shows its own messages.
- **Held items with reactions**: Plasma Core, Dream Pillow, a changed Leftovers and **Philosopher Stone** (the holder survives any hit and recovers its HP).
- **Team Runtime Data** (Pokémon > Tools): changes the HP, the stats and the status condition of the team, refreshes the Pokémon and heals them.
- The ability and its flag stay in agreement when you change the species, the form or the ability.
- New tutorials: `docs/tutorials/14-add-a-type.md` and `docs/tutorials/15-add-an-item.md`.

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
