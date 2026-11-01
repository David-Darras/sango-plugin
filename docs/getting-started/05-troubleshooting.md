# 5. Troubleshooting

Find your problem in the list. Do the steps of the solution.

## Build problems

### `Please set DEVKITARM in your environment`

The terminal does not know where devkitPro is.

- **Windows:** use the devkitPro shell (`C:\devkitPro\msys2\msys2_shell.bat`), not the Windows command prompt.
- **Linux:** close the terminal and open a new terminal. If the problem continues, type `source /etc/profile.d/devkit-env.sh`.

### `CTRPluginFramework.hpp: No such file or directory`

libctrpf is not installed. Do [step 3 and step 4 of the installation](01-install-the-tools.md) again.

### `3gxtool: command not found`

3gxtool is not installed. Type `pacman -S 3gxtool` (Windows) or `sudo dkp-pacman -S 3gxtool` (Linux).

### `pacman` does not find `libctrpf` or `3gxtool`

1. Make sure that `pacman.conf` contains the lines of ThePixellizerOSS.
2. Type `pacman -Sy` (Windows) or `sudo dkp-pacman -Sy` (Linux).
3. Install the packages again.

### `No rule to make target` or a path error

The path of the project folder contains a space. Move the folder to a path without spaces.

### A strange error after you rename or move a file

The build folders contain old files. Type `make clean`, then `make`.

### `static assertion failed: ... must match ...`

A game structure does not have the correct size. You changed a structure in a `native/` folder.
Make sure that the size of each member is correct. Read [Game structures](../concepts/game-structures.md).

## Run problems

### The screen "The ROM hack has N problem(s)"

The library checks your ROM hack when the game starts. A few seconds after the
start, the top screen shows the problems that it found. Press **A** or **B** to
close the list. The log of the plugin (**Plugin > Log**) also shows them.
**Plugin > Problems** shows the list again.

| Problem | Cause | Solution |
|---|---|---|
| `move N: the id is used two times` | Two moves (or abilities, items, scripts, types) have the same id. | Give each one its own id. |
| `move N: more than 128 new moves` | The list is full. The library ignores the move. The lists keep 128 moves, 64 abilities, 128 items, 256 C++ scripts and 128 characters. | Remove a move. |
| `move N: a move of the game has this id` | The id is not free. The new move replaces the name of a move of the game. | Use an id above `MoveId::kCount`. For an ability, above `AbilityId::kCount`. |
| `item N: use an id from 800 to 1023` | The id is not an item of the game and not a free id. | Use an id from 800 to 1023 for a new item. |
| `script N: use an id from 30500 to 59999` | A C++ script has the id of a script of the game. | Use a free id: see [Scripts](../concepts/scripts.md). |
| `character N: script N is not registered` | A character of `MapCharacter::Add()` has a free script id without a C++ script. | Call `NativeScript::Register()` with this id. |
| `type N: no icon in sdmc:/sango/types` | The icon file of the type is not on the SD card. The game shows the icon of Fairy. | Copy `<name>.bclim` to `sdmc:/sango/types/`. |
| `item N: no icon in sdmc:/sango/items` | The icon file of the item is not on the SD card, and the item has no `icon_item`. | Copy `<name>.bclim` to `sdmc:/sango/items/`, or set `icon_item`. |
| `trainer N: more than 256 teams (use SetTable)` | `TrainerTeams::Add()` keeps 256 teams. | Use `TrainerTeams::SetTable()` for more teams. |
| `character on map N: more than 128 characters` | `MapCharacter::Add()` keeps 128 characters for all the maps. | Remove a character. |
| `characters: N models, use 16 at most` | The game loads the models of all your characters in each region, with the models of the region (33 at most). Too many models stop the game. | Use the same models for more characters. |

### The menu does not open

1. Make sure that the plugin loader is enabled (Azahar: **Emulation > Configure > System**; console: Rosalina menu).
2. Make sure that the plugin is in `luma/plugins/000400000011C500/`.
3. Make sure that there is only one `.3gx` file in this folder.
4. Press **Start**. If you changed the menu buttons, press your buttons.

### The game stops (black screen or freeze) at the start

1. Make sure that the game is Alpha Sapphire **version 1.4**.
2. Make sure that you built for the correct game. `make` builds for Alpha Sapphire. `make GAME=XY` builds for Pokémon X.
3. Remove your last change and build again. A hook with a wrong address or a wrong signature stops the game.

### The game stops when I open a page

The page reads memory that does not exist yet. Some pages work only in one part of the game.
For your own pages, use `app.CheckProcess(...)`. Read [Add a menu page](../tutorials/02-add-a-menu-page.md).

### The game crashes while the menu is open

While the menu is open, the plugin writes the file `sdmc:/sango/frame_trace.txt`
three times each second. After a crash, read this file on the SD card. It shows
the last page and the last entry of the menu, the number of texts and
rectangles of the frame, the size of the command list of the GPU, and the free
memory with its lowest value. A command list that is almost full, or a memory
that is almost empty, points to the cause.

## Find the cause of a problem

- Use the log window. Add `ui::Log::Print(u"value: %d", value);` to your code.
  Open **Plugin > Log** in the menu to read the log.
- Remove changes one at a time. Build and test after each change.
- On Azahar, the log console of the emulator shows the crashes.
  Enable it in **Emulation > Configure > Debug**.
