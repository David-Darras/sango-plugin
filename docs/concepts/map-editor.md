# The map editor

The map editor changes the **attributes of the tiles** of a world with the
stylus. An attribute tells the game what a tile does: a wall, water, a tile
where wild Pokémon appear, the type of the ground. The editor works with
Alpha Sapphire and Omega Ruby only.

A **world** is the set of tiles that the game keeps for a part of Hoenn. The
tiles have coordinates in their world, and the editor keeps one file for each
world.

## Open the editor

1. Go to the overworld.
2. Open **Overworld > Map Editing > Map Editor**, then **Open The Map Editor**.
3. For the result on the top screen, turn on the
   [world overlay](world-overlay.md).

The bottom screen shows the map from above, one square for each tile:

| Color | Tile |
|---|---|
| Dark | Wall. |
| Blue | Water. |
| Green | Wild Pokémon. |
| Light blue, sand or gray | The type of the ground. |
| Black | The tile is not loaded. |

A red frame shows the player. A white frame shows the last tile that you
touched. A thin line shows the border of the blocks of 40 tiles.

## The tools

The button at the top of the right column changes the tool.

| Tool | What it does |
|---|---|
| Paint | The tiles under the stylus change. The brush is a square of 1, 2, 3, 5 or 9 tiles (the buttons **Size-** and **Size+**). |
| Rect | Drag a rectangle. The tiles change when you release the stylus. A rectangle has 200 tiles per side at most. |
| Fill | The tiles that touch the tile and have the same attribute change. The fill changes 4000 tiles at most. |
| Pick | Copies the whole attribute of a tile. The attribute becomes **Stamp**: the next strokes paint this exact attribute. |
| Move | Drag to scroll the map. |

The D-pad also scrolls the map. L and R (or **Zoom-** and **Zoom+**) change the
size of the tiles. **Home** centers the map on the player.

## The attributes

The second button changes the attribute that the tool gives to the tiles.

The game does not store "wall", "water" and "wild Pokémon" in the tile. It
stores a **ground number**, and it looks the three flags up in a table with
this number. So the attributes Wall, Walkable, Water and Wild + / - change the
ground number of the tile to a number that has the flags that you want (for
example 1 for a wall, 0 for a plain tile, 148 for water, 31 for grass). The
type of the ground changes with it. The other attributes change single bits of
the tile and keep its ground number.

| Attribute | Effect |
|---|---|
| Wall | The player cannot walk on the tile. |
| Walkable | The tile is not a wall and not water. |
| Water | The tile is water, and not a wall. |
| Wild + / Wild - | Wild Pokémon can appear (+) or not (-). |
| Block Tr + / - | The tile blocks the trainers (+) or not (-). |
| Deco + / - | A decoration can stand on the tile (+) or not (-). |
| Ground, Footstep, Battle BG | Gives the number **Val** to the type of the ground, the sound of the steps or the background of the battle. |
| Stamp | The attribute that **Pick** copied. |

The buttons **Val-** and **Val+** change the number. Hold the button to go
fast. The top screen shows the name of the ground and of the footstep for the
touched tile, so touch a tile of the kind that you want, then read its number.

## The mode Events

The button **Mode** (at the bottom of the right column) switches between the
modes Tiles and Events. In the mode Events, the map shows a frame on each
event: cyan for a sign, magenta for a trigger, lime for a warp, orange for a
trainer and blue for another character. A yellow frame shows the selected
event. The top screen lists the fields of the selected event. See
[the signs and the triggers](map-events.md) for the meaning of the fields.

The tools of the mode Events:

| Tool | What it does |
|---|---|
| Select | Touch an event to select it. A character wins over a warp, a warp over a trigger, a trigger over a sign. |
| Place | Touch a tile to add an event there. The button under the tool gives the kind: sign, trigger or warp. The new event has no script. |
| Move | Touch a tile to move the selected event there. Drag to move it with the stylus. |
| Remove | Touch an event that you added to remove it. The events of the game stay. |

The buttons **Field-** and **Field+** choose a field of the selected event,
**Val-** and **Val+** change it (hold the button to go fast), and **Step**
changes the size of a change (1, 10, 100 or 1000). A field has a minimum and a
maximum value, and the editor keeps the value inside.

A character is read when the map loads. After you change a character (its
tile, its direction, its script, its movement or its trainer range), touch
**Reload**: the game loads the map again and the character uses the new
values. A trainer sees `Trainer range` tiles in front of it.

**Save** writes the events of the map to `sdmc:/sango/map_events.txt` (the
tiles keep their own file). The editor also writes the events when you close
it, if you changed an event. A warp that leads to a map or an exit that does
not exist can freeze the game: change the destination with care, and use the
numbers of the maps and exits of the game.

## Undo and save

**Undo** and **Redo** cancel and repeat a whole stroke: a line of the brush, a
rectangle or a fill. The editor keeps 1536 changed tiles of history.

The editor writes the changes of a world to
`sdmc:/sango/maps/layout_<number>.tiles` when you touch **Save**, when you
close the editor and when you go to another world (the page of the menu has a
switch **Save When Leaving**). The game reads the file when you enter the
world, and the changes come back.

The editor keeps the changes as **runs**: a run is a row of tiles with the same
attribute, so a rectangle costs one run for each row. A world has 1536 runs. If
the editor is full, it shows **Full: save**.

## The music, the weather and the time of a map

Open **Map Editor > Music, Weather And Time Of This Map**. Turn on **Change
The Music**, **Change The Weather** (Alpha Sapphire only) or **Change The
Time**, and choose a value. The game applies the settings each time that it
loads the map, and gives back the old values when the player goes to a map
without settings. The weather can need a few seconds to show. **Save To The SD
Card** writes `sdmc:/sango/zones.txt`.

## The scripts without code

Open **Map Editor > Scripts Without Code** to write scripts with steps (show a
text, give an item, set a flag...). A sign, a trigger or a character that has
the number 32000 to 32011 starts them. See [the scripts without code](data-scripts.md).

## Text files and live reload

The page of the map editor has three ways to edit on a computer:

- **Export The Tiles To Text** writes the 128 x 128 tiles around the player to
  `sdmc:/sango/maps/layout_<number>.txt`: `#` is a wall, `~` water, `,` a tile
  with wild Pokémon, `.` a plain tile, `?` a tile that is not loaded. The
  line `origin X Z` gives the tile of the top left corner.
- **Import The Tiles From Text** reads the file and changes the tiles whose
  character is different. An undo cancels the import.
- **Live Reload Of The Files** reads the files again each time that they
  change: the text file of the tiles, `map_events.txt` and `scripts.txt`. Open
  the file on a computer, save it, and the game changes. The check runs one
  time every 0.75 second.

## What the editor cannot do

- It cannot get the original attribute of a tile back with the eraser. Use
  **Undo**, or **Clear All The Changes Of This World** in the page of the menu:
  it deletes the file and loads the map again.
- It does not change the 3D ground, the buildings or the doors, only the
  attributes. A wall does not hide anything: it blocks the player.
- The old tile editor with the D-pad (in the page **World Layout**) keeps its
  own changes in another file. Use one editor for a tile.
