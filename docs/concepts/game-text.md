# The texts of the game

All the texts of the game are in text files: the dialogues, the names of the
moves and the items, the menus. The game reads a text with one function (the
one that `sys::Message::GetString()` calls). `core::TextEditor` looks at the
result of this function. It keeps the last texts that the game read, and it
replaces a text when the player changed it.

## The menu

Open **Data > Game Text**.

1. The list shows the last texts that the game read, the newest first. A star
   means that the text is changed. **Texts seen** counts all the texts that the
   plugin saw: 0 means that the hook does not work.
2. To change a dialogue, close the menu, talk to the character, then open the
   menu again: the dialogue is at the top of the list.
3. Open a text. The keyboard of the menu holds 31 characters, so the text is
   in **parts** of about 26 characters (never cut inside a marker). Change a
   part with the keyboard; the editor joins the parts again. The last part is
   empty: write in it to add text at the end. The change is for the next time
   that the game reads the text, so talk to the character again.
4. **Restore Original** gives the text of the game again.
5. **Save To The SD Card** writes the changes to `sdmc:/sango/texts.txt`. The
   game reads the file when it starts. The file keeps 48 texts at most.

A text can have 160 characters at most, and it must fit in the room that the
game gives to the text.

## The markers

The text of the game has codes between the letters: a tag is the code `0x0010`,
the number of the next codes, the code of the tag, then its parameters. The
editor shows a tag as a marker between brackets.

| Marker | What it does |
|---|---|
| `[nl]` | A new line (the character 10). |
| `[line]` | Changes the page: the next text scrolls up by one line (tag `BE00`). |
| `[page]` | Changes the page: the box is cleared before the next text (tag `BE01`). |
| `[wait:N]` | Stops the text for N frames (tag `BE02`). |
| `[red]` `[green]` `[blue]` `[orange]` `[pink]` `[purple]` `[cyan]` `[yellow]` `[sky]` | Changes the color of the next letters (tag `BD00` and the number of the color, from 0 to 8). |
| `[/color]` | The next letters have the normal color (tag `BD01`). |
| `[tag:HHHH]` or `[tag:HHHH:N:N]` | Any other tag: 4 hexadecimal digits for the code, then the parameters in decimal. For example `[tag:0100:0]` is a name that the game puts in the text. |
| `[x:HHHH]` | A control character that is not a tag. |
| `[lb]` `[rb]` | The characters `[` and `]`. |

The nine colors are the ones of the message window of the game. A color needs
`[/color]` to stop.

The **Insert** entries add a marker at the end of the text. To put a marker in
the middle, edit the whole text with the keyboard. The **Custom Marker**
entries build the marker of any tag: the group and the index are the two bytes
of the code (189 and 0 are `BD00`, the color).

A wrong marker is refused, with a sentence, and the text does not change.

## The texts of the scripts

A dialogue that a script shows is not in a file of the game: the script engine
builds it in memory. The list shows it as **Script**. The editor cannot name it
by a number, so it names it by a hash of its content: a change is for all the
texts that have this exact content. If two characters say the same sentence,
both change.

## What the editor does not do

- It does not change the speed of the text or the size of the letters.
- It does not make new colors: the game has nine colors in its tags.
- A text that the game read before the plugin started is not in the list.
