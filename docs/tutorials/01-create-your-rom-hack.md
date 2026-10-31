# Tutorial 1: Create your ROM hack

In this tutorial, you make your own product: `myhack`.
At the end, you have the file `myhack-release.3gx`. In the game, all the
wild Pokémon are shiny, and the menu shows your own page.

**Time:** 10 minutes.
**You need:** a working build. See [Build the plugin](../getting-started/02-build-the-plugin.md).

## Step 1: Make the product

In the project folder, type:

```bash
python tools/new_hack.py myhack --author "Your name" --summary "My ROM hack"
```

> **Note:** Use `python3` if `python` does not work. The name contains only
> lowercase letters, digits and `_`.

The script makes these files:

```
myhack/
  myhack.plgInfo      the information of the plugin
  include/            your headers
  src/entrypoint.cc   your settings and your menu
```

The `Makefile` finds the product: each folder with a `.plgInfo` file and a
`src/` folder is a product. You do not change the `Makefile`.

## Step 2: Read the plugin information file

Open `myhack/myhack.plgInfo`. 3gxtool reads it to make the `.3gx` file.

```yaml
Author: Your name

Version:
    Major: 0
    Minor: 1
    Revision: 0

Targets:
    #- 0x0011C500

Title: myhack

Summary: My ROM hack

Description: |
  My ROM hack. A ROM hack for Pokemon Alpha Sapphire v1.4, made with Sango Plugin.

Compatibility: Any

MemorySize: 5MiB

UsePrivateMemory: false
```

You can change `Author`, `Version`, `Summary` and `Description`.

> **Note:** Do not change `MemorySize`. The plugin needs 5 MiB.

## Step 3: Write your settings

Open `myhack/src/entrypoint.cc` and replace its contents with this code:

```cpp
/**
 * @file entrypoint.cc
 * @brief The settings and the menu of my ROM hack.
 */

#include "sango.h"

// The library calls this function one time, when the game starts.
void SetUpHack() {
  // All the Pokémon are shiny (1 chance out of 1).
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
}

// The first page of the menu: the page that the player sees first.
void LoadHackMenu(ui::MainApplication& app, void* args) {
  app.Add("Shiny rate", pokemon::Shiny::GetInstance().rate)
     .Add("Battle", ui::LoadBattlePage);
}
```

`sango.h` includes the features of the library. Include it in each file of
your ROM hack.

The library calls the functions of your product (see `lib/include/hack.h`):

| Function | When the library calls it | Without the function |
|---|---|---|
| `SetUpHack()` | One time, when the game starts. | The game does not change. |
| `LoadHackMenu()` | When the player opens the menu. | The menu shows all the pages of the library. |
| `UpdateHack()` | One time for each frame. | Nothing more happens at each frame. |

> **Note:** Write the names exactly as in the table. If a name is not correct,
> the library does not call the function.

## Step 4: Build

In the devkitPro shell, type:

```bash
make myhack
```

The project folder now contains `myhack-release.3gx`.

## Step 5: Test

1. Copy `myhack-release.3gx` to `luma/plugins/000400000011C500/` on the SD card.
   Remove the other `.3gx` files of this folder.
2. Start the game.
3. Walk in the tall grass. The wild Pokémon are shiny.
4. Press **Start**. The menu shows your page with two entries.

> **Tip:** With `config.mk`, `make run-myhack` builds, copies and starts the game.

## What you learned

- `tools/new_hack.py` makes a product: a folder with a `.plgInfo` file and a `src/` folder.
- `SetUpHack()` sets the features. `LoadHackMenu()` makes the first page of the menu.
- The settings of a feature are normal variables (`Shiny::GetInstance().rate`).
- `make <name of the folder>` builds the product.

## Next steps

- Look at `kaizo/src/`: a complete ROM hack. It writes its own `Initialize()`
  function: see [Architecture](../concepts/architecture.md#your-own-initialize).
- [Tutorial 2: Add a menu page](02-add-a-menu-page.md)
