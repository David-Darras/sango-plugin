# Tutorial 17: Translate your ROM hack

In this tutorial, you translate the texts of your ROM hack into French. A
player with a French game sees the French texts. The other players see the
English texts.

**Time:** 10 minutes.
**You need:** [Tutorial 4](04-add-a-move.md) or
[Tutorial 9](09-write-an-overworld-script.md): a new move or a script with
texts.

## How it works

You write the texts of your ROM hack in English, in the code (`u"..."`). A
translation file gives the same texts in an other language. When the game
shows a text of your ROM hack, the library looks for its translation in the
language of the game.

The library translates:

- the names and the descriptions of the new moves, abilities, types and items,
- the texts of the C++ scripts (`Talk()`, `ShowMessage()`, `Choose()`).

A translator does not need the code: the translation is a text file.

## Step 1: Make the translation file

Make the file `translations/fr.txt` in the data folder of your ROM hack
(`sdmc:/sango/myhack/translations/fr.txt`). Save it in UTF-8.

```text
# The translations of my ROM hack into French.
# Each line: the English text, " => ", then the translation.

# The moves
Thunderclap => Coup de tonnerre
A fast electric attack.\nIt calls the rain. => Une attaque électrique rapide.\nElle appelle la pluie.

# The scripts
Hello, Trainer!\nDo you like candies? => Bonjour, Dresseur !\nTu aimes les bonbons ?
Here are 5 Rare Candies! => Voici 5 Super Bonbons !
```

- The English text must be the same as in the code, letter for letter.
- `\n` is a new line, as in the code.
- A line that starts with `#` is a comment.

## Step 2: Test

1. Copy the file to the SD card.
2. Start a game in French.
3. Look at your move in the summary, or talk to your character. The texts are
   in French.

A text without a translation stays in English.

## The languages

| Language of the game | File |
|---|---|
| Japanese | `translations/ja.txt` |
| English | `translations/en.txt` |
| French | `translations/fr.txt` |
| Italian | `translations/it.txt` |
| German | `translations/de.txt` |
| Spanish | `translations/es.txt` |
| Korean | `translations/ko.txt` |

The file of a language has 32 KB at most.

## Translations in the code

You can also give a translation in the code, in `SetUpHack()`:

```cpp
core::Translations::Add(Language::kFrench, u"Thunderclap",
                        u"Coup de tonnerre");
```

## Values in the texts

The texts of the scripts can show values. Write their names between braces:

| Name | Value |
|---|---|
| `{player}` | The name of the player. |
| `{money}` | The money of the player. |
| `{var:N}` | The variable N of the ROM hack (see [Scripts](../concepts/scripts.md#the-flags-and-the-variables-of-the-rom-hack)). |

```cpp
s.Talk(u"Hello, {player}! You have {money} Pokédollars.");
```

The translations can use the same names:

```text
Hello, {player}! You have {money} Pokédollars. => Bonjour, {player} ! Tu as {money} Pokédollars.
```

## What you learned

- The texts of the ROM hack are in English in the code.
- `translations/<language>.txt` in the data folder translates them.
- `{player}`, `{money}` and `{var:N}` show values in the texts of the scripts.

**Next:** [Tutorial 18: Make a ROM hack without code](18-make-a-rom-hack-without-code.md)
