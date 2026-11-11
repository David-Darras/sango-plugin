Sango Plugin {{VERSION}} for Pokémon Alpha Sapphire v1.4 (no ROM patch).

**New in {{VERSION}}**

- **Fixed: a black screen at the first start of a new game.** Without a save file, the game stayed black after the choice of the language, and the introduction with the Professor never started.
  The plugin changed the table of the species while the game loaded its data again: the game then freed a table that was not its own. The plugin now waits for the overworld or a battle.
- **The trainers of Pokémon X and Y in your battles.** Thirteen 3D models of X and Y are now in Alpha Sapphire: Professor Sycamore, Lysandre (two models), the Team Flare admins and grunts (male and female),
  Aliana, Xerosic, Celosia, Bryony, Mable and AZ. A trainer shows its model in the battle with **the introduction of X and Y**: the animated background, the particles and the camera of the game.
  The menu page **Battle > Trainer Models** has the switch **XY Trainer Models**.
- **The class of a trainer.** `battle::TrainerClasses::Set` gives a trainer another class (the title, the 2D picture, the 3D model and the music of the class).
  The list of the 280 classes of the game is in `trainer_class.h`.
- **The names of the lists are now files in `sango/names`** (the zip has them): they are not in the plugin any more, and the plugin reads them only while a menu needs them.
- **A 2D picture for the player.** **Battle > Trainer Models > Player Picture** gives the player the 2D picture of any class (also the classes of Pokémon X and Y), with the 2D introduction of the game, and no 3D model.
- **A model for the player.** **Battle > Trainer Models > Player Model** gives the player any of these models in the battles: the 13 models of X and Y, and the models of the game (Zinnia, Steven, Maxie, Archie, Wally...).
  Each model plays its own introduction when the player throws the first Poké Ball, with its effects and its camera.
  The Poké Ball and the Mega Evolution use the motions of the model, so the model is not in T-pose any more.
- **The 3D introduction is forced** when a trainer has a model, also if the class of the trainer shows a 2D picture.
- The trainer models are a new optional pack (see **Install**). Without it the plugin works as before.
- **The tall grass of Pokémon X and Y.** Five kinds of grass (the plain grass, flowers and tall grass) that you place by hand.
  Menu page **Overworld > Tall Grass > Grass Of X And Y**: choose the kind, then **Place In Front** (a rectangle of 1 to 8 tiles each side) or **Place 3x3 Square**.
  The grass sways when the player walks into it. The map keeps the grass in a file, and **Light Set** changes its light. The pack `grass.pc` is optional (see **Install**).
- **The battle backgrounds of Pokémon X and Y.** The page **Battle > Static Encounter** has four lists: **Ground**, **Far Ground**, **Background** and **Plate**. Each list has the piece of the game, the pieces of ORAS and the pieces of X and Y
  (`ORAS - Grass`, `XY - Cave`...), so you can mix them in the battle that the page starts. The pack of the backgrounds is optional (see **Install**).

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
   - Optional, the trainer models of Pokémon X and Y (the models, their introductions and their motions): {{TRAINER_MODELS_URL}}
     → unzip in `sango/trainer_model/` (the pack has the folders `sequence` and `layout`)
   - Optional, the tall grass of Pokémon X and Y (one file, `grass.pc`, not in an archive): {{GRASS_URL}}
     → copy the file to `sango/grass/grass.pc`
   - Optional, the battle backgrounds of Pokémon X and Y: {{BACKGROUNDS_URL}}
     → unzip in `sango/` (the pack has the folders `battle_background` and `names`)
3. Azahar: Emulation > Configure > System > Enable 3GX Plugin Loader.
4. Start the game, and press Start in the overworld to open the menu.

Do not save the game with a new Pokémon in your party: this can corrupt your save file.

The zip has a `README.txt` with the same steps.

Feedback and bug reports are welcome.
