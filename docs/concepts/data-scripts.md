# The scripts without code

A **script** is a list of steps that the game runs when the player presses
{A} on a sign, walks in a trigger or talks to a character. The scripts of the
game are made with the Pawn language, and the C++ scripts need a build. The
scripts without code are written in the menu, and they work at once.

The plugin has 12 of them, with the numbers **32000 to 32011**. Give one of
these numbers to a sign, a trigger or a character (see
[the signs and the triggers](map-events.md) and [the map editor](map-editor.md)).

## The menu

Open **Overworld > Map Editing > Map Editor > Scripts Without Code**.

1. Open a script. It has 8 steps.
2. For each step, choose the **Action**, then fill the **Parameter 1**, the
   **Parameter 2** and the **Text** if the action needs them.
3. **Add A Sign Here** and **Add A Trigger Here** put an event with this
   script on your tile. Walk away and test it.
4. **Save To The SD Card** writes `sdmc:/sango/scripts.txt`.

The steps run in order. The action **End** stops the script: the steps after
it do not run.

## The actions

| Action | Parameter 1 | Parameter 2 |
|---|---|---|
| Say | 0: the window of a sign. 1: the window of a person. | Not used. |
| Ask Yes/No | Not used. The answer No stops the script. | Not used. |
| Give Item | The number of the item. | How many (0 is 1). The step does nothing when the bag is full. |
| Add Money | The amount. | 0 gives the money, 1 takes it. |
| Set Flag | The number of the flag. | Not used. |
| Reset Flag | The number of the flag. | Not used. |
| Require Flag | The number of the flag. The script stops when it is not set. | Not used. |
| Play Sound | The number of the sound. | Not used. |
| Wait Frames | The number of frames (60 are one second). | Not used. |
| Run Game Script | The number of a script of the game. | Not used. |
| Give Pokemon | The number of the gift in the table of the game. | Not used. |

**Say** shows the text with the same markers as the
[game text editor](game-text.md): `[red]`, `[/color]`, `[nl]`, `[wait:30]`... A
text has two parts of 31 characters, because the keyboard of the menu holds 31
characters. The editor joins the two parts. A wrong marker shows nothing.

A flag that **Set Flag** sets stays in the save file. Use a flag number that
the story does not use.

> **Warning:** **Run Game Script** with a number that the game does not know
> can freeze the game.

## Example: a sign that gives a Potion

1. Step 1: Say, with the text `You find a Potion!`.
2. Step 2: Give Item, with the number of the Potion.
3. Step 3: Play Sound, with the number of a sound of the game.

A script cannot test that a flag is **not** set, and it cannot change a work
of a trigger. A script that gives an item gives it each time that the player
starts it.

## The file

Each line of `scripts.txt` is `script step action parameter1 parameter2 text`.
The text is UTF-8 and has the markers. Edit the file on a computer, then use
the switch **Live Reload Of The Files** in the page of the map editor: the
scripts change as soon as you save the file.
