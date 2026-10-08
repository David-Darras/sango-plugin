# 3. Run the plugin

This guide loads your `.3gx` file with the game.
You can use the Azahar emulator or a real 3DS console with Luma3DS.

## The game

| Game | Version | Title id | Status |
|---|---|---|---|
| Pokémon Alpha Sapphire | 1.4 | `000400000011C500` | Supported. All the products work. |
| Pokémon X | 1.5 | `0004000000055D00` | Experimental. Only the overlay. Build with `make GAME=XY`. |

The version is important. The plugin uses fixed memory addresses.
With a different version, the addresses are wrong and the game stops.

Use your own copy of the game. To dump your cartridge and the update,
follow the [3DS Hacks Guide](https://3ds.hacks.guide/dumping-titles-and-game-cartridges.html).

## The plugin folder

The plugin must be in this folder of the SD card:

```
luma/plugins/<title id>/
```

For Alpha Sapphire: `luma/plugins/000400000011C500/`.

Put **one** `.3gx` file in this folder. You can give it any name.
The project uses the name `sango_plugin.3gx`.

---

## On the Azahar emulator

### Step 1: Install the game and the update

1. Open Azahar.
2. Select **File > Install CIA...** and select the CIA file of the update 1.4.
3. Add the folder of your game dump to the game list.

### Step 2: Enable the plugin loader

1. Select **Emulation > Configure**.
2. Select the **System** tab.
3. Select **Enable 3GX Plugin Loader**.
4. Click **OK**.

### Step 3: Copy the plugin

1. Select **File > Open Azahar Folder**.
2. Open the folder `sdmc`.
3. Make the folders `luma/plugins/000400000011C500` if they do not exist.
4. Copy your `.3gx` file into this folder.

> **Tip:** `make run-overlay` does steps 3 and 4 for you.
> See [Build the plugin](02-build-the-plugin.md#optional-build-copy-and-start-the-game-with-one-command).

### Step 4: Start the game

Start Alpha Sapphire from the game list.
The plugin starts with the game.

---

## On a 3DS console

You need a console with [Luma3DS](https://github.com/LumaTeam/Luma3DS) v13.0 or later.
Luma3DS v13.0 and later have a plugin loader.

### Step 1: Copy the plugin

1. Turn off the console.
2. Put the SD card in your computer.
3. Make the folders `luma/plugins/000400000011C500` if they do not exist.
4. Copy your `.3gx` file into this folder.
5. Put the SD card back in the console.

### Step 2: Enable the plugin loader

1. Turn on the console.
2. Press **L + Down + Select** to open the Rosalina menu.
3. Select **Plugin Loader** to enable it.
4. Press **B** to close the menu.

The plugin loader stays enabled after a restart.

### Step 3: Start the game

Start Alpha Sapphire from the HOME Menu.

---

## Use the menu

The menu of the plugin is on the top screen.

| Button | Action |
|---|---|
| **Start** | Opens or closes the menu. |
| **Up / Down** | Selects an entry. |
| **Left / Right** | Changes the value of the entry. |
| **A** | Opens the page of the entry, or runs the entry. |
| **B** | Goes back to the previous page. |
| **X** | Sets the value that you typed on the bottom screen (overlay only). |
| **L + R** | Shows or hides the log window. |

You can change the button that opens the menu: **Plugin Theme > Key 1, Key 2, Key 3**.

Some pages work only in one part of the game.
For example, the **Overworld** page works only when the player walks in the overworld.
If the part of the game is wrong, the page does not open and you hear an error sound.

---

**Next step:** [4. Set up your editor](04-set-up-your-editor.md)
