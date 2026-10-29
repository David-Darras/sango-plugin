Sango Plugin {{VERSION}}
by ZettaD
https://github.com/David-Darras/sango-plugin

A plugin for Pokemon Alpha Sapphire v1.4. It changes the game while it runs:
you do not patch the game, and you share no game file.

You need
--------
- Pokemon Alpha Sapphire with the update 1.4 (title id 000400000011C500).
  Use your own copy of the game.
- Luma3DS v13.0 or later on a 3DS console, or the Azahar emulator.
- A program that opens .7z files, for the packs (for example 7-Zip).

What is in this zip
-------------------
luma/plugins/000400000011C500/sango_plugin.3gx   The plugin.
README.txt                                       This file.

Install the plugin
------------------
1. Unzip the zip at the root of the SD card.
   Azahar on Windows:  %APPDATA%/Azahar/sdmc/
   A 3DS console:      the root of the SD card.
   You get luma/plugins/000400000011C500/sango_plugin.3gx .
   Put only one .3gx file in this folder.
2. Azahar only: Emulation > Configure > System > Enable 3GX Plugin Loader.
   Console only: press L + Down + Select to open the Rosalina menu, select
   "Plugin Loader" to enable it, then press B.
3. Start Alpha Sapphire. The plugin starts with the game.
4. In the overworld, press Start to open the menu.

Install the packs
-----------------
The packs are big, so they are not in this zip. The .7z files of the packs
have NO folder inside: they hold the files directly. So you must make the
folder yourself, then unzip the pack INSIDE this folder.

The folders go in the "sango" folder of the SD card:
  Azahar on Windows:  %APPDATA%/Azahar/sdmc/sango/
  A 3DS console:      sango/ at the root of the SD card.
Make the "sango" folder if it does not exist.

1. Pokemon models (the 3D models of the Pokemon, needed for the new Pokemon):
     {{POKEMON_MODELS_URL}}
   Make the folder  sango/pokemodel/  and unzip the pack in it.
   You must get files like sango/pokemodel/index.bin and sango/pokemodel/0.raw .

2. Overworld models (the 3D models of the characters, needed for the new
   characters):
     {{OVERWORLD_MODELS_URL}}
   Make the folder  sango/overworld_model/  and unzip the pack in it.
   You must get files like sango/overworld_model/index.bin .

3. Music (the music of Pokemon X and Y, optional):
     {{MUSIC_URL}}
   Make the folder  sango/bgm/  and unzip the pack in it.
   You must get sango/bgm/list.txt and the music files next to it.

If a file is in sango/ and not in its folder, the plugin does not find it.
Without the packs the plugin works, but the new Pokemon, the new characters
and the extra music are missing.

First steps
-----------
- Sound > Lock Music keeps the music that plays now.
- Overworld > World Overlay draws the walls, the water, the warps and the
  hidden items on the ground, in 3D.
- Save Data > Team edits the Pokemon of your team.
- Do not save the game with a new Pokemon in your team: this can corrupt
  your save file.

Problems
--------
- The game stops at the start: the game version must be 1.4.
- Nothing happens: check that the plugin loader is on and that the .3gx
  file is the only one in the folder.
- Report a bug: https://github.com/David-Darras/sango-plugin/issues

The plugin is free software (GPL-3.0). The models and the music of the packs
come from other games: they are not part of the plugin.
