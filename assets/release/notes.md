Sango Plugin {{VERSION}} for Pokémon Alpha Sapphire v1.4 (no ROM patch).

**New in {{VERSION}}**

- **Following Pokémon** (Cheats > Followers > Following Pokemon): your first Pokémon walks near you in the overworld.
  It has the light of the overworld (day and night), an idle animation, and it plays battle and Pokémon-Amie animations by itself.
  L plays a battle animation and R plays a Pokémon-Amie animation, both with the cry.
  Pokémon Omega Ruby and Alpha Sapphire have no walk animation, so the walk is made in code on the bones of the model:
  two legs for a biped, four legs in diagonal pairs for a quadruped, many legs for a spider. A Pokémon that flies or floats does not walk.
  It goes away cleanly for the menus, the doors, the scripts, the battles and the map changes.
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
