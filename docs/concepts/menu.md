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
| **Left / Right** | Changes the value. Hold the button: a number changes 10 times faster. |
| **A** | Opens the page, runs the action, or switches On / Off. |
| **Y** | Pins the entry to the quick access, or removes the pin. |
| **X** | Opens the pinned entries in a circle (see below). In the numpad and the keyboard: applies the value. |
| **B** | Goes back to the previous page. |

The menu does not use SELECT, HOME, ZL or ZR: SELECT and START are the same
button on some consoles, and ZL / ZR do not exist on a standard 3DS.

The bottom screen (`ui::MainAppPainter`) shows:

- the path of the open pages and the description of the selected entry.
  On the first page, the path line shows the name, the version and the
  author of the plugin, and the date of the build;
- a touch editor for the value: On / Off buttons, a grid of the texts of
  the entry (12 at most), the numpad, or the keyboard;
- the current process and the current event of the game;
- five large buttons at the bottom edge: previous section, next section,
  pin, search, back. Each button also shows its key;
- after a change, an **Undo** button in the top-right corner. It puts the
  old value back. A second touch puts the new value back. It works until
  the player leaves the page.

**Search**: the Search button opens a page with a text and a list of
results. The results show while the player types: the menu reads all the
pages (the pages that need a different part of the game are not in the
results). A on a result opens its page and selects the entry.

**The circle of the pins**: X shows the 8 pinned entries in a circle, one
in each direction (pin 1 up, then clockwise). Hold X, push a direction,
release X: the page of the pin opens with the entry selected. The player can
also push a direction and press A, or touch a pin. B closes the circle.
With practice, the player remembers the directions and does not look.

The **numpad** has the layout of a phone (1 2 3 at the top), a minus key and
a decimal point. It starts with the current value: the first key replaces
the value, DEL edits it. The -1 / +1 / -10 / +10 buttons are next to the
keys, and the limits of the value are at the right of the text bar.

The **keyboard** shows only the characters that the font of the game can
draw. < and > show the previous and the next page. << and >> jump to the
previous and the next group of characters: Latin, Greek, Hiragana,
Katakana, Kanji, the icons of the game... It starts with the current text.

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
| `WithReadOnly()` | The player sees the value of the last entry, but cannot change it. |
| `OnChange(function)` | Runs the function after each change of a value on the page. |

### Pages that change a copy of the data

Some data needs a step after a change. For example, the Pokémon of the save
data are encrypted: the page changes a decrypted copy, then the copy must be
encrypted and written back. Give this step to `OnChange`. The menu runs it
after each change, so the player never needs a "Save" entry:

```cpp
void LoadMyPokemonPage(ui::MainApplication& app, void* args) {
  app.OnChange(SavePokemon)   // Encrypts the copy and writes it back.
     .Add("Level", copy.level)
     .Add("Shiny", copy.is_shiny);
}
```

The menu runs `OnChange` before it builds the page again (`WithRefresh`):
the page then reads the new data. A pinned or a recent entry keeps the
`OnChange` function of its page.

A pinned entry is a copy of the entry: the player changes its value directly
on the first page. When the page of the entry needs a part of the game
(`CheckProcess`), the copy works only in this part of the game.

The menu keeps the way to each pinned or recent entry as a text: the names of
the pages, the section in brackets, then the name of the entry. For example:
`Battle > Settings > [Display] Type Helper`. When the first page loads, the
menu runs the page functions without the screen to find the entries again.
So a pin needs page names (`Open(..., title)`): the menu gives them
automatically for the entries of type page.

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
- **Changes apply at once** (`OnChange`): the player does not need to
  remember a "Save" step.
- **The editors start with the current value**: the player edits a value,
  and does not type it again.
- **Few levels of pages**: a page with sections is faster than several small
  pages. The Save Data page shows all its parts in four sections.
- **A circle for the 8 pins**: a radial menu is fast for 8 items at most:
  each item has its own direction, and the hand learns it. It is not good
  for long lists, so the pages stay lists.
- **Undo**: the player can try a value without fear.
- **The values are in a column** on the top screen: the eyes find them
  faster. A long name keeps its value on the same line.
- **Short animations** (about 100 ms) show where the cursor goes and if a
  page opens or closes. The Plugin Theme page can stop them.

## The settings file

The menu keeps its settings in `sdmc:/sango/menu.ini`: the effects, the
colors, the sounds, the buttons that open the menu, the pinned entries and
the recent entries. The menu reads the file when the game starts, and writes
it when the menu closes, only when a setting changed. The player can change
the file in a text editor:

```ini
[menu]
text_shadow = 1
animations = 1

[theme]
background = 000000BF
text = FFFFFFFF
selected_text = FF1A80FF
menu_key_1 = None

[pins]
pin1 = Battle > Settings > [Display] Type Helper
pin2 = [Play] Game Speed
```

The colors are `RRGGBBAA` in hexadecimal. Other features can use the same
format with `core::Ini` (`lib/include/core/ini.h`).

## The types of entries

The type of the second parameter selects the type of the entry:

| Second parameter | The entry | The player |
|---|---|---|
| `bool&` | Shows On or Off. | Changes it with A, Left / Right or the touch screen. |
| `u8&`, `u16&`, `u32&`, `s32&`, `f32&`... | Shows a number. | Changes it with Left / Right, or types it on the bottom screen. |
| An `enum class` variable | Shows the number of the value. | Changes it with Left / Right. |
| `c16* text, size` | Shows a text. | Types it on the keyboard. |
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
| `WithCallback(function)` | Runs the function when the player presses **A** on the entry. |
| `WithReadOnly()` | The player cannot change the value. |
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

**Keep the number of texts small.** Each `DrawText()` adds many commands to
the command list of the GPU, and this list has a fixed size: about 100 texts
in one frame (both screens) make the game crash. `sys::Graphics` counts the
texts of each frame: it stops the shadows at 80 texts (`kShadowLimit`) and
the texts at 88 (`kTextLimit`). To use few texts, draw one text for a full
line: `sys::Graphics::AppendSpaces()` adds spaces up to a width, then
`AppendText()` adds the next column. The menu draws each row of keys like
this. Use it only for short gaps: each space adds a small error. Draw a button with one rectangle, not
`DrawRectStroke()` (four rectangles).

**Background images.** The menu shows `sdmc:/sango/menu_top.tga` and
`sdmc:/sango/menu_bottom.tga` behind its pages (Plugin Theme > Background
Image). Save them as TGA without compression (24 or 32 bits): 400 x 240
pixels for the top screen, 320 x 240 for the bottom screen. A smaller image
shows in the center of the screen. Image Opacity sets the opacity of the
image, and the color of the theme covers it: its alpha sets how much of the
image shows.
`ui::Image` draws any other TGA image of 512 x 512 pixels at most. Call
`Image::Prepare()` while the plugin draws the top screen: a new texture while
the game draws the bottom screen makes the game crash. To make a
TGA image from a PNG image:

```bash
python tools/png_to_tga.py menu_top.png
```

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
