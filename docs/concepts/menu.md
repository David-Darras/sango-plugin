# The menu

The plugin has its own menu on the top screen. This page explains how it works.

## Pages and entries

The menu shows one **page** at a time. A page is a list of **entries**.

A page is a function. The menu calls it each time it shows the page:

```cpp
void LoadMyPage(ui::MainApplication& app, void* args) {
  app.Add("Game Speed", core::GameSpeed::GetInstance().game_speed)
     .WithBounds(1, 4)
     .Add("Battle", ui::LoadBattlePage)
     .Add("Heal the party", HealParty);
}
```

`app.Add(...)` adds one entry. It returns `app`, so you can add the next entry
on the next line.

## The types of entries

The type of the second parameter selects the type of the entry:

| Second parameter | The entry | The player |
|---|---|---|
| `bool&` | Shows On or Off. | Changes it with Left / Right. |
| `u8&`, `u16&`, `u32&`, `s32&`, `f32&`... | Shows a number. | Changes it with Left / Right, or types it on the bottom screen. |
| An `enum class` variable | Shows the number of the value. | Changes it with Left / Right. |
| A page function | Shows the name. | Opens the page with **A**. |
| A function `void(void*)` | Shows the name. | Runs the function with **A**. |
| `CheatCodeId` | Shows On or Off. | Enables or disables a cheat code. |
| Nothing | Shows the name only. | Nothing. |

Special entries:

| Function | The entry |
|---|---|
| `AddSpecies(name, variable)` | Shows the name of a species. |
| `AddMove(name, variable)` | Shows the name of a move. |
| `AddItem(name, variable)` | Shows the name of an item. |
| `AddAbility(name, variable)` | Shows the name of an ability. |
| `AddType(name, variable)` | Shows the name of a type. |
| `AddSeparator()` | Shows an empty line. |

## Options of an entry

These functions change the last entry that you added:

| Function | What it does |
|---|---|
| `WithBounds(min, max)` | Sets the minimum and the maximum value. |
| `WithMin(min)`, `WithMax(max)` | Sets one limit. |
| `WithArray(texts, count)` | Shows a text for each value (0 = first text). |
| `WithCallback(function)` | Runs the function when the value changes. |
| `WithFactor(factor)` | Changes the step of a decimal value. |
| `WithRefresh()` | Builds the page again when the value changes. |

Example with texts:

```cpp
static const c8* kModes[] = {"Off", "Slow", "Fast"};
app.Add("Mode", my_mode).WithArray(kModes, SIZE(kModes));
```

## Pages that need a part of the game

Some data exists only in one part of the game. For example, the map data
exists only in the overworld. Start these pages with `CheckProcess`:

```cpp
void LoadMyOverworldPage(ui::MainApplication& app, void* args) {
  if (app.CheckProcess(overworld::address::kVtable)) return;
  // The player is in the overworld: the page can read the map data.
}
```

If the game is in a different process, `CheckProcess` closes the page,
plays an error sound and returns `true`.

## The limits

- A page has 64 entries at most.
- The menu shows 15 entries at a time. The player scrolls to see the others.
- The menu opens 8 pages inside each other at most.

## The look of the menu

A **painter** (`ui::Painter`) draws the menu. The overlay uses
`ui::MainAppPainter`. The kaizo product has its own painter
(`kaizo::Painter`) with a different look.

To change the look, make a class that inherits `ui::Painter`.
Give it to `plugin::OpenMenu(painter, root_page)`.

## The pages of the library

The library has many pages. Their list is in `lib/include/ui/page/pages.h`.
Your product can use them in its own menu:

```cpp
app.Add("Camera", ui::LoadOverworldCameraPage)
   .Add("Apps", ui::LoadAppPage);
```

## Related pages

- [Add a menu page](../tutorials/02-add-a-menu-page.md)
- The code: `lib/include/ui/main_application.h`, `lib/src/ui/page/`
