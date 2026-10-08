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

## Find the cause of a problem

- Use the log window. Add `ui::LogApplication::Print(u"value: %d", value);` to your code.
  Press **L + R** in the game to read the log.
- Remove changes one at a time. Build and test after each change.
- On Azahar, the log console of the emulator shows the crashes.
  Enable it in **Emulation > Configure > Debug**.
