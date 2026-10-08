# Tutorial 2: Add a menu page

In this tutorial, you add a page to the menu of your product.
The page has a switch, a number, a list of texts, an action and a sub-page.

**Time:** 15 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).
**Read also:** [The menu](../concepts/menu.md).

## Step 1: Make the page

Make the file `myhack/src/my_pages.cc`:

```cpp
/**
 * @file my_pages.cc
 * @brief The menu pages of my ROM hack.
 */

#include "common.h"
#include "overworld/address.h"
#include "savedata/native/pokemon_team.h"
#include "ui/main_application.h"

namespace myhack {

// The values that the page shows. They are static: they keep their value
// when the page closes.
static bool s_is_hard_mode = false;
static u8 s_level_cap = 20;
static u8 s_difficulty = 1;

static const c8* kDifficultyNames[] = {"Easy", "Normal", "Hard"};

// An action: the menu calls it when the player presses A on the entry.
static void HealParty(void* args) {
  savedata::PokemonTeam::GetInstance().HealAllPokemons();
}

// A sub-page. It reads the party, so it needs the overworld.
static void LoadPartyPage(ui::MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;

  app.Add("Heal the party", HealParty);
}

// The page.
void LoadMyPage(ui::MainApplication& app, void* args) {
  app.Add("Hard mode", s_is_hard_mode)
     .Add("Level cap", s_level_cap)
     .WithBounds(1, 100)
     .Add("Difficulty", s_difficulty)
     .WithArray(kDifficultyNames, SIZE(kDifficultyNames))
     .AddSeparator()
     .Add("Party", LoadPartyPage);
}

} // namespace myhack
```

## Step 2: Declare the page

Make the file `myhack/include/myhack/my_pages.h`:

```cpp
#pragma once

namespace ui {
class MainApplication;
}

namespace myhack {
/// The page of the options of my ROM hack.
void LoadMyPage(ui::MainApplication& app, void* args);
} // namespace myhack
```

## Step 3: Add the page to the root page

In `myhack/src/entrypoint.cc`:

1. Add the include at the top:

   ```cpp
   #include "myhack/my_pages.h"
   ```

2. Add one entry to `LoadRootPage`:

   ```cpp
   void LoadRootPage(ui::MainApplication& app, void* args) {
     app.Add("Shiny rate", pokemon::Shiny::GetInstance().rate)
        .Add("Battle", ui::LoadBattlePage)
        .Add("My options", myhack::LoadMyPage);
   }
   ```

## Step 4: Build and test

```bash
make myhack
```

In the game:

1. Press **Start** to open the menu.
2. Select **My options** and press **A**.
3. Change **Difficulty** with **Left / Right**: the text changes.
4. Go to **Party** in the overworld. Select **Heal the party** and press **A**.
5. Press **B** to go back.

## Use the values

The menu changes the variables directly. Use them in your code:

```cpp
if (s_is_hard_mode) {
  // Make the battles more difficult.
}
```

To use a value in a different file, move the variable to a header, or add a
function that returns it.

## Add a page to the library

When a page is useful for all the products, add it to the library:

1. Write the page in `lib/src/ui/page/page_<family>.cc` (for example `page_battle.cc`).
2. Declare it in `lib/include/ui/page/pages.h`.
3. Add one entry in the root page of the family (for example `LoadBattlePage`).

## What you learned

- A page is a function that calls `app.Add(...)` for each entry.
- The type of the variable selects the type of the entry.
- `WithBounds` and `WithArray` change the last entry.
- `CheckProcess` protects a page that needs a part of the game.

**Next:** [Tutorial 3: Add a hook](03-add-a-hook.md)
