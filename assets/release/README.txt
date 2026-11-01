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
For the players:
  luma/plugins/000400000011C500/sango_plugin.3gx   The plugin.
  sango/types/, sango/items/                       The icons of the plugin.

For the modders (make a ROM hack without code, see the end of this file):
  sango_maker/luma/plugins/000400000011C500/sango_maker-release.3gx
                                                   The Sango Maker plugin.
  sango_maker/sango/maker/                         The example text files.
  maker_editor/index.html                          The editor of the text
                                                   files, for a computer.

README.txt                                         This file.

=====================================================================
PART 1: FOR THE PLAYERS
=====================================================================

Install the plugin
------------------
1. Unzip the zip at the root of the SD card. The folders sango_maker and
   maker_editor are for the modders: you can delete them from the SD card.
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
- Cheats > Followers > Following Pokemon: your first Pokemon walks near
  you. L plays a battle animation and R a Pokemon-Amie animation, with its
  cry.
- Do not save the game with a new Pokemon in your team: this can corrupt
  your save file.

Problems
--------
- The game stops at the start: the game version must be 1.4.
- Nothing happens: check that the plugin loader is on and that the .3gx
  file is the only one in the folder.
- Report a bug: https://github.com/David-Darras/sango-plugin/issues

=====================================================================
PART 2: FOR THE MODDERS (SANGO MAKER)
=====================================================================

Sango Maker is a second plugin. It reads text files on the SD card and
changes the game: the wild Pokemon, the trainers, the species and the
scripts of the characters. You do not write code and you do not build a
plugin. The menu of Sango Maker also has all the editors of Sango Plugin.

Install Sango Maker
-------------------
The game loads one plugin only. Sango Maker replaces Sango Plugin.

1. On the SD card, open luma/plugins/000400000011C500/ and remove
   sango_plugin.3gx (keep a copy if you want to play with it again).
2. Copy the CONTENT of the folder sango_maker/ of this zip to the root of
   the SD card. You get:
     luma/plugins/000400000011C500/sango_maker-release.3gx
     sango/maker/encounters.txt, trainers.txt, species.txt, story.txt,
     characters.txt, rules.txt, shops.txt, items.txt, moves.txt
3. Start Alpha Sapphire. Sango Maker reads the files of sango/maker/ when
   the game starts.

The text files
--------------
encounters.txt   The wild Pokemon of the maps.
trainers.txt     The teams, the names, the AI and the items of the trainers.
species.txt      The stats, the types, the abilities, the TMs, the
                 learnsets and the evolutions of the species.
story.txt        The scripts: messages, choices, gifts, battles, warps.
characters.txt   The characters and the trainers to put on the maps.
rules.txt        The rules of the battles, the multipliers of the
                 experience, the catch rate and the money, the shiny rate
                 and the starters.
shops.txt        The items and the Pokemon that the shops sell.
items.txt        The price, the power of Fling, the HP and the EVs of the
                 items.
moves.txt        The type, the category, the power, the accuracy, the PP
                 and the priority of the moves.

Open them in a text editor (Notepad is good) and save them in UTF-8. The
examples show the format. A mistake in a file shows on a screen of problems
when the game starts: the screen gives the file and the line.

Test without a restart: open the menu, then Sango Maker. "Read The Files
Again" reads the files at once and lists their problems. "Live Reload" reads
them when they change on the SD card. The page also changes the flags and the
variables of story.txt.

The editor (on a computer)
--------------------------
Open maker_editor/index.html in a web browser (double-click). Keep names.js
in the same folder. The editor works without internet. It checks the lines
of the files while you write. Click "Download" to save a file, then copy
it to sango/maker/ on the SD card.

Share your ROM hack
-------------------
Your ROM hack is the file sango_maker-release.3gx and the folder sango/maker.
Give the two to the players, with the steps of "Install Sango Maker".

The full guide (tutorial 18):
https://github.com/David-Darras/sango-plugin/blob/master/docs/tutorials/18-make-a-rom-hack-without-code.md

=====================================================================

The plugin is free software (GPL-3.0). The models and the music of the packs
come from other games: they are not part of the plugin.
