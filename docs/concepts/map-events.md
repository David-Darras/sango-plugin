# The signs and the triggers of the map

A character is not the only thing that starts a script. A map also has
**signs** and **triggers**: zones of tiles that start a script.

| Event | When the script starts |
|---|---|
| Sign | The player presses {A} in front of the zone. |
| Trigger | The player walks in the zone, or pushes against it. |

The map has two more tables: the warps (they change the map) and the
interrupt triggers (the game checks them at each frame). The editor does not
change these two tables.

## The menu

Open **Overworld > Map Events**.

1. The page lists the signs and the triggers of the current map. A plus means
   an event that you added. A star means a change of an event of the game. The
   numbers are the tile and the script.
2. Open an event to change it. The change is live: walk to the zone to test it.
3. **Add A Sign Here** and **Add A Trigger Here** add an event on your tile,
   at your height. The new event has no script (65535): it does nothing until
   you give it one.
4. **Move To Player** puts the zone of an event on your tile.
5. **Save To The SD Card** writes the changes of the map to
   `sdmc:/sango/map_events.txt`. The game reads the file when the map loads.
   The file keeps 64 events for all the maps.
6. **Reset This Map** gives the map the events of the game again.

The editor does not remove an event of the game. To disable one, give it the
script 65535, or move it to a place where nobody goes.

> **Warning:** A script number that the game does not know can freeze the
> game when the event starts. Use the number of a script of the game, or a
> number that your C++ script registers (see the tutorials about scripts).

## The fields

A **sign** has:

- **Script**: the number of the script to start.
- **Tile X**, **Tile Z**, **Width**, **Depth**: the zone, in tiles.
- **Height**: the height of the ground. The zone works for a player within
  one tile above or below this height.
- **Talk From**: the side where the player must stand.
- **Kind**: 0 is a normal sign. 1 works only from the front. The numbers 2 to
  10 need the player to look at the zone (boards, trees, circles).

A **trigger** has the same zone, and:

- **When**: 0 starts the script when the player walks in the zone. 5 to 8
  start it when the player pushes against the zone, up, down, left or right.
  9 to 12 are the four diagonals.
- **Condition Work** and **Value**: with a work of 0, the trigger always
  works. With another number, the trigger works only when this work of the
  game has the value. The story flags are works.

## The file

Each line is `map kind index fields`. The kinds are `S+` (added sign), `T+`
(added trigger), `S=` and `T=` (change of the event of the game with this
number). A sign has `x z width depth height script talk_from kind`. A trigger
has `x z width depth height script work value when`.

## What the editor does not do

- It does not make a script. It starts the scripts that exist: those of the
  game, and those that you add with the C++ scripts.
- It leaves a map alone when this map has more than 40 signs or 40 triggers.
- The list shows the first 20 signs and the first 20 triggers.
