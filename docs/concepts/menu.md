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

## The controls

| Button | What it does |
|---|---|
| **START** | Opens and closes the menu (the Plugin Theme page can change it). |
| **Up / Down** | Selects an entry. Hold the button to go faster. A new press at the end of the page goes back to the start. |
| **L / R** | Goes to the previous / next section. A page without sections moves one screen. |
| **Left / Right** | Changes the value. |
| **A** | Opens the page, runs the action, or switches On / Off. |
| **Y** | Pins the entry to the quick access, or removes the pin. |
| **X** | Applies the number of the numpad. |
| **B** | Goes back to the previous page. |

The menu does not use SELECT, HOME, ZL or ZR: SELECT and START are the same
button on some consoles, and ZL / ZR do not exist on a standard 3DS.

The bottom screen (`ui::MainAppPainter`) shows:

- the path of the open pages and the description of the selected entry;
- a touch editor for the value: On / Off buttons, a grid of the texts of
  the entry (12 at most), or the numpad and the -10 / -1 / +1 / +10 buttons;
- four large buttons at the bottom edge: previous section, next section,
  pin, back. Each button also shows its key.

## Sections, descriptions and quick access

```cpp
void LoadMyPage(ui::MainApplication& app, void* args) {
  app.AddQuickAccess()   // The pinned entries and the recent entries.
     .AddSection("Play")
     .Add("Game Speed", core::GameSpeed::GetInstance().game_speed)
     .WithDescription("1 is the normal speed.")
     .AddSection("Advanced")
     .Add("Save Data", ui::LoadSaveDataPage);
}
```

| Function | What it does |
|---|---|
| `AddSection(name)` | Adds a section title. L and R jump between the sections. |
| `WithDescription(text)` | Sets the text of the bottom screen for the last entry. Use one or two short sentences. |
| `AddQuickAccess()` | Adds the pinned entries (Y, 8 at most) and the 4 recent entries. Use it at the start of the first page. |

A pinned entry is a copy of the entry: the player changes its value directly
on the first page. When the page of the entry needs a part of the game
(`CheckProcess`), the copy works only in this part of the game. The pins and
the recent entries stay until the game closes.

### Why the menu works like this

- **Sections** (L / R) reduce the number of entries to look through.
- **Pinned entries in a fixed zone** at the top are faster than a long
  list. The menu never moves the other entries: the player keeps the
  positions in memory.
- **Large touch buttons near the edges** are faster to reach than many
  presses of the +Control Pad.
- **Choices that the player sees** (a grid of texts) are faster than values
  that the player must remember.
- **The description** of the selected entry tells what it does, without a
  manual.

## The types of entries

The type of the second parameter selects the type of the entry:

| Second parameter | The entry | The player |
|---|---|---|
| `bool&` | Shows On or Off. | Changes it with A, Left / Right or the touch screen. |
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
| `AddSeparator()` | Shows a line. L and R also jump between separators. |
| `AddSection(name)` | Shows a section title. |

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
