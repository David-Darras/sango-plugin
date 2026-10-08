# Tutorial 1: Create your ROM hack

In this tutorial, you make your own product: `myhack`.
At the end, you have the file `my_hack-release.3gx`. In the game, all the
wild Pokémon are shiny, and the menu shows your own page.

**Time:** 15 minutes.
**You need:** a working build. See [Build the plugin](../getting-started/02-build-the-plugin.md).

## Step 1: Make the folders

In the project folder, make these folders:

```
myhack/
  include/
  src/
```

## Step 2: Make the plugin information file

Make the file `myhack/my_hack.plgInfo`. 3gxtool reads it to make the `.3gx` file.

```yaml
Author: Your name

Version:
    Major: 0
    Minor: 1
    Revision: 0

Targets:
    #- 0x0011C500

Title: my-hack

Summary: My ROM hack

Description: |
  My first ROM hack for Pokémon Alpha Sapphire.

Compatibility: Any

MemorySize: 5MiB

UsePrivateMemory: false
```

> **Note:** Do not change `MemorySize`. The plugin needs 5 MiB.

## Step 3: Make the entry point

Make the file `myhack/src/entrypoint.cc`:

```cpp
/**
 * @file entrypoint.cc
 * @brief The entry point of my ROM hack.
 */

#include "plugin.h"
#include "pokemon/patch/shiny.h"
#include "ui/main_application.h"
#include "ui/painter.h"
#include "ui/page/pages.h"

namespace {

// The root page of the menu: the first page that the player sees.
void LoadRootPage(ui::MainApplication& app, void* args) {
  app.Add("Shiny rate", pokemon::Shiny::GetInstance().rate)
     .Add("Battle", ui::LoadBattlePage);
}

// Sets the features of the library for this ROM hack.
void InstallCallbacks() {
  // All the Pokémon are shiny (1 chance out of 1).
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
}

// The plugin calls this function one time for each frame.
void EveryFrame() {
  plugin::UpdateFrame();
  plugin::DrawFrame();
}

} // namespace

// The plugin calls this function one time, when the game starts.
void Initialize() {
  plugin::InitializeEngine();
  InstallCallbacks();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), LoadRootPage);
  plugin::Start(EveryFrame);
}
```

The name `Initialize` is important: the library calls a function with this
exact name. It must be outside of all namespaces.

## Step 4: Add the product to the Makefile

Open the `Makefile`.

1. Find the product blocks (`# sango_kaizo.3gx: the ROM hack`).
2. Add this block after the last product block:

   ```makefile
   # my_hack.3gx: my ROM hack
   myhack_TARGET	:=	my_hack
   myhack_SOURCES	:=	$(LIB_SOURCES) myhack/src
   myhack_INCLUDES	:=	$(LIB_INCLUDES) myhack/include
   myhack_PSF		:=	myhack/my_hack.plgInfo
   ```

3. Find the line `GAME_PRODUCTS := overlay kaizo undertow`.
4. Add `myhack` at the end:

   ```makefile
   GAME_PRODUCTS := overlay kaizo undertow myhack
   ```

The name before `_TARGET` (`myhack`) is the name of the product.
The value of `_TARGET` (`my_hack`) is the name of the file.

## Step 5: Build

In the devkitPro shell, type:

```bash
make myhack
```

The project folder now contains `my_hack-release.3gx`.

## Step 6: Test

1. Copy `my_hack-release.3gx` to `luma/plugins/000400000011C500/` on the SD card.
   Remove the other `.3gx` files of this folder.
2. Start the game.
3. Walk in the tall grass. The wild Pokémon are shiny.
4. Press **Start**. The menu shows your page with two entries.

> **Tip:** With `config.mk`, `make run-myhack` builds, copies and starts the game.

## What you learned

- A product is a folder with an entry point and a `.plgInfo` file.
- `Initialize()` starts the library, sets the features and opens the menu.
- The settings of a feature are normal variables (`Shiny::GetInstance().rate`).
- The `Makefile` has one block for each product.

## Next steps

- Look at `kaizo/src/entrypoint.cc`: a complete ROM hack.
- [Tutorial 2: Add a menu page](02-add-a-menu-page.md)
