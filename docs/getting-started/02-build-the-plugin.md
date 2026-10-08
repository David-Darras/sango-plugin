# 2. Build the plugin

This guide downloads the source code and builds the `.3gx` files.
Do [1. Install the tools](01-install-the-tools.md) first.

## Step 1: Download the source code

Open the devkitPro shell (Windows) or a terminal (Linux).

Go to the folder where you keep your projects. For example, on Windows:

```bash
cd /c/
```

Download the source code:

```bash
git clone https://github.com/David-Darras/sango-plugin.git
```

Go into the new folder:

```bash
cd sango-plugin
```

> **Note:** Use a folder path without spaces. `make` does not work well with spaces.

## Step 2: Build

Type:

```bash
make
```

The first build takes some minutes. The next builds are faster:
`make` compiles only the files that you change.

> **Tip:** `make -j8` compiles 8 files at the same time. It is faster on most computers.

When the build is complete, the project folder contains these files:

| File | Product | What it is |
|---|---|---|
| `sango_plugin-release.3gx` | overlay | All the pages of the library in one menu. Use it to explore the game. |
| `sango_kaizo-release.3gx` | kaizo | The ROM hack Pokémon Sango Kaizo. |
| `sango_undertow-release.3gx` | undertow | The ROM hack Pokémon Undertow (work in progress). |

The `.elf` files are the same programs before `3gxtool`. You do not need them.

## Other build commands

| Command | What it does |
|---|---|
| `make overlay` | Builds only the overlay. Also `make kaizo` or `make undertow`. |
| `make GAME=XY` | Builds for Pokémon X v1.5 (experimental, overlay only). |
| `make clean` | Removes all the build output. Use it when a build has a strange error. |
| `make run-overlay` | Builds the overlay, copies it to the SD card of the emulator and starts the game. See below. |

## Optional: build, copy and start the game with one command

`make run-<product>` needs the paths of your emulator and of your game.
You write these paths in the file `config.mk`. Git ignores this file:
your paths stay on your computer.

1. Copy `config.mk.example` to `config.mk`:

   ```bash
   cp config.mk.example config.mk
   ```

2. Open `config.mk` in a text editor.
3. Change the paths. Each line has a comment that explains it.
4. Save the file.
5. Type:

   ```bash
   make run-overlay
   ```

The command builds the overlay. Then it copies the plugin to
`<SDMC>/luma/plugins/<title id>/sango_plugin.3gx`. Then it starts the emulator.

## How the build works

You do not need to know this to use the project. Read it when you add a product.

- The `Makefile` builds one plugin for each product.
- Each product uses the sources of `lib/` and the sources of its own folder.
- Each product has its own build folder: `release-overlay/`, `release-kaizo/`...
- The game of the build is a define: `-DGAME_ORAS` or `-DGAME_XY`.
  The macro `GAME_ADDRESS(xy, oras)` uses it to select the addresses.
- The language is C++11 (`-std=gnu++11`) without exceptions and without RTTI.
  The plugin runs inside the memory of the game, so it must stay small.

---

**Next step:** [3. Run the plugin](03-run-the-plugin.md)
