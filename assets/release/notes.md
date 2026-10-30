Sango Plugin {{VERSION}} for Pokémon Alpha Sapphire v1.4 (no ROM patch).

**New in {{VERSION}}**

- **Fly on a Pokémon** (Cheats > Flight > Fly On A Pokemon): you sit on a flying Pokémon (Staraptor by default) and fly with the Circle Pad.
  R goes up and L goes down. No wild Pokémon and no collision while you fly.
- **Ride a Pokémon** (Cheats > Flight > Ride A Pokemon): you sit on the back of a Pokémon with four legs (Raikou by default) and ride it on the ground.
  The game moves you as usual, so the collisions, the doors and the wild Pokémon stay. The legs of your character are bent to the sit pose,
  and your arms hold the mane. Any species can be ridden, the seat is set in Cheats (height, back, leg spread, speed) and a table in the code
  (`ride_offset.h`) gives the seat of a species.
- **New types** (`battle::GameExtension::AddType()`): add a type, or change the name and the icon of a type of the game, with its own type chart.
  The overlay has two examples, **Cosmic** and **Sound**. Their icons are in `sango/types/` of the zip (the release script makes them from the PNG files of `assets/types`).
- **A natural walk for the quadrupeds**: the legs step one after the other, like a cat, with the knees bent, the feet on the ground and a small bounce of the back,
  the head and the tail. When you run, a quadruped gallops like a cheetah: the two front legs go together, and the two hind legs go together.
- **Following Pokémon** (Cheats > Followers > Following Pokemon): your first Pokémon walks near you in the overworld.
  It has the light of the overworld (day and night), an idle animation, and it plays battle and Pokémon-Amie animations by itself.
  L plays a battle animation and R plays a Pokémon-Amie animation, both with the cry.
  Pokémon Omega Ruby and Alpha Sapphire have no walk animation, so the walk is made in code on the bones of the model.
  A Pokémon that flies or floats does not walk. It goes away cleanly for the menus, the doors, the scripts, the battles and the map changes.
- The fields of `MoveData` now have enum classes (`MoveCategory`, `DamageCategory`, `MoveTarget`, `MoveStat`, `EffectTurnType`).

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

Feedback and bug reports are welcome.
