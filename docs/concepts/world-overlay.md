# The world overlay

The overlay draws colors on the ground of the overworld, on the top screen. It
shows what the game knows about the tiles and the events around the player.
It works with Alpha Sapphire and Omega Ruby only.

Open **Overworld > World Overlay**, turn on **Enabled**, and close the menu.
Turn on **Legend** to see the list of the colors on the screen.

## How it draws

The overlay finds the camera of the overworld, and projects the corners of the
tiles on the screen with its matrices. The library of the game draws only
rectangles that stay parallel to the screen. A tile in perspective is a
trapezoid, so the overlay draws it as a pile of thin horizontal strips. The
tiles of the same color in a row are one shape, which needs fewer strips.

The strips get thicker when there are many of them, so that the number of
drawings stays near 450. **Strips Of The Last Frame** shows the real number.
A big radius with many colored tiles slows the game: lower the radius, or turn
off the layers that you do not need. The overlay does not draw while the menu
is open.

The drawing is flat: it uses the height of the player for all the tiles, and
it does not hide the colors behind a building.

## The layers

| Layer | Color | What it shows |
|---|---|---|
| Walls | Red | The player cannot walk on the tile. |
| Water | Blue | The player can surf on the tile. |
| Wild Pokémon | Green | Wild Pokémon can appear on the tile. |
| Blocks Trainers | Orange | The tile blocks the trainers. |
| Decoration Spots | Purple | A decoration can stand on the tile. |
| Plain Tiles | Pale yellow | A tile that the player can walk on and that has no other color. |
| Signs | Cyan | The zone where {A} starts a script. |
| Triggers | Magenta or pink | The zone where a script starts when the player walks in (magenta) or pushes against it (pink). |
| Warps | Lime | The zones that change the map. |
| Trainer Sight | Orange | The tiles where a trainer sees the player. |

A tile that has several of these attributes shows the first color of the
table. A yellow square shows a hidden item. A white square shows the player.
A square on a character is orange for a trainer and blue for another
character.

**Script Numbers** writes the number of the script over the zones and the
characters, 20 numbers at most. The zones come from the map event editor (see
[the signs and the triggers](map-events.md)): the overlay shows also the
events that you added.

## The line of sight of the trainers

A trainer does not see in a cone. The game tests a line of tiles, one tile
wide, in front of the trainer. The line is as long as the range of the
trainer, and it stops at the first wall. Some trainers see one tile in the
four directions. The overlay draws this line for the characters that have a
trainer behavior. The range is the first parameter of the character.

The kinds of trainer are numbered from the order of the list of the game. This
numbering is not checked on a console. If a trainer has no orange line, the
kind of the character is probably not in the list.

## The state

The page shows counters to find a problem:

- **Scene Draws Seen**: how many scenes the game drew. 0 means that the
  overlay does not find the draw of the game.
- **Top Cameras Found** and **Cameras Refused**: the cameras that have the
  shape of the top screen, and the others.
- **Frames With A Camera**: only counts while the overlay is enabled.
- **Screen Pivot**: the default is Not Turned, the value that works with the
  camera of the game. Change it when the colors are turned by 90 degrees.

The defaults are a radius of 8 tiles and an opacity of 80 percent.
