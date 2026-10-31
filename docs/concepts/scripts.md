# Scripts

A **script** controls an event of the overworld: a conversation, a gift, a
cutscene, a trainer battle. This page explains the scripts of the game and the
C++ scripts of the plugin.

## The scripts of the game

The game scripts are programs in the **Pawn** language. They are `.amx` files
in an archive of the game. Each script has an id (`script::ScriptId`).

A script calls **natives**: C functions of the game. For example:

| Native | What it does |
|---|---|
| `ItemAdd` | Gives an item. |
| `FlagGet`, `FlagSet` | Reads or sets an event flag. |
| `WorkGet`, `WorkSet` | Reads or sets a script variable. |
| `TalkMdlMsg_Seq` | Shows a message in a speech bubble. |

The names of the natives are the names that the game uses.

## C++ scripts

With the plugin, you write a script in C++. The game runs it like a game script.

```cpp
#include "script/patch/native_script.h"

void MyGreeter(script::Context& s) {
  s.TalkStart();                     // The character turns to the player.
  s.Talk(u"Hello! Welcome to my ROM hack.");
  if (s.AskYesNo()) {
    s.Talk(u"Here is a present for you.");
    s.GiveItem(ItemId::kRareCandy, 5);
    s.PlayJingle(script::kJingleItem);
  }
  s.TalkEnd();                       // The character goes back to normal.
}
```

The function receives a `script::Context`. The context has the functions of the script.

### How it works

1. You register the function with an id: `script::NativeScript::Register(id, function)`.
2. When the game starts this script id, the library gives the game a very small `.amx` file.
3. This file calls one native again and again. This native runs your C++ function.
4. When your function waits (for a key press, a message, an animation), it gives control back to the game.
   The game draws the next frame. Then your function continues.

Your function looks like normal code, but it runs over many frames.

### The script ids

The ids from `ScriptId::kFirstCustom` (30500) to `ScriptId::kLastCustom` (59999)
are free for C++ scripts. The library uses some of them. Use `HackScript()`:
it gives the ids 31000 to 31999, that the library never uses.

```cpp
constexpr ScriptId kMyGreeter = script::HackScript(0);
constexpr ScriptId kMyShop = script::HackScript(1);
```

Do not use an id of the game: the game script does not run any more.

### Give the script to a character

A script runs when the player talks to a character.
`overworld::MapCharacter::Add()` puts a new character on a map with your script:

```cpp
overworld::MapCharacterRequest request;
request.map_id = MapId::kLittlerootTown;
request.model_id = ModelId::kResearcherMale;
request.script_id = kMyGreeter;
request.tile_x = 47;
request.tile_z = 77;
request.facing = overworld::Facing::kDown;
overworld::MapCharacter::Add(request);
```

You can add 128 characters at most, on all the maps. A map shows 32
characters at most: the characters of the game first. Use 16 different
models at most for your characters.

## The functions of the context

| Function | What it does |
|---|---|
| `TalkStart()`, `TalkEnd()` | Starts and ends a conversation with the character. |
| `Talk(text)` | Shows a message and waits for the player to press a button. |
| `ShowMessage(text)`, `CloseMessage()` | Shows a message without a wait. Closes it. |
| `AskYesNo()` | Shows Yes / No. Returns `true` for Yes. |
| `GiveItem(item, count)`, `HasItem(item)` | Gives an item. Checks the Bag. |
| `GivePokemon(gift_id)` | Gives a gift Pokémon. |
| `SelectPokemon()` | Opens the party. Returns the selected slot. |
| `GetFlag(flag)`, `SetFlag(flag)`, `ResetFlag(flag)` | Reads and changes an event flag. |
| `GetHackFlag(n)`, `SetHackFlag(n)`, `ResetHackFlag(n)` | Reads and changes a flag of the ROM hack (0 to 4095). |
| `GetHackVariable(n)`, `SetHackVariable(n, value)` | Reads and changes a variable of the ROM hack (0 to 511). |
| `GetVariable(id)`, `SetVariable(id, value)` | Reads and changes a script variable. |
| `GetMoney()`, `AddMoney(amount)`, `SubMoney(amount)` | Reads and changes the money. |
| `PlaySound(id)`, `PlayJingle(id)` | Plays a sound or a short music. |
| `Walk(object, direction, count)`, `Face(object, direction)` | Moves a character. |
| `Wait(frames)` | Waits some frames. |
| `CallScript(id)` | Runs a game script. |
| `TrainerBattle(trainer)` | Starts a battle against a trainer. Returns the result. |
| `WildBattle(species, level)` | Starts a battle against a wild Pokémon (Alpha Sapphire only). |
| `WildBattle(encounter)` | Starts a battle against a row of the static encounter table. |
| `GivePokemon(species, level)` | Gives a Pokémon. A full party sends it to the PC. |
| `HealParty()` | Heals all the Pokémon of the party. |
| `Choose({u"A", u"B"})` | Shows a list of choices. Returns the index of the choice. |
| `Warp(map, x, z, height)` | Sends the player to a map. The script continues there. |
| `FadeOut()`, `FadeIn()` | Makes the screens black, then shows them again. |
| `StartEventMusic(music)`, `EndEventMusic()` | Plays the music of an event, then the music of the map again. |
| `CallNative("Name", ...)` | Calls a native of the game with its name. |

The full list is in `lib/include/script/patch/context.h`.

### Battles

`TrainerBattle()` and `WildBattle()` wait for the end of the battle. They return
a `script::BattleResult`: `kWon`, `kLost`, `kRan`, `kFoeRan` or `kCaught`.

```cpp
void Rival(script::Context& s) {
  s.TalkStart();
  s.Talk(u"Let's battle!");
  s.TalkEnd();
  // A trainer of lib/include/battle/constant/trainer.h. battle::TrainerTeams
  // can give it a new team.
  s.TrainerBattle(TrainerId::kRoute103Brendan1);
  s.Talk(u"You won!");
}
```

> **Note:** When the player loses, the player wakes up at the last Pokémon
> Center and the game ends the script: the next lines do not run. With
> `TrainerBattleOption::kCannotLose`, the script continues after a loss.

### Choices

Show the question with `ShowMessage()`, then the choices with `Choose()`:

```cpp
s.ShowMessage(u"Which Pokémon do you want?");
const s32 choice = s.Choose({u"Bulbasaur", u"Charmander", u"Squirtle"});
s.CloseMessage();
if (choice == script::kChoiceCancel) return;  // The player pressed B.
const SpeciesId kStarters[] = {SpeciesId::kBulbasaur, SpeciesId::kCharmander,
                               SpeciesId::kSquirtle};
s.GivePokemon(kStarters[choice], 5);
```

### The other natives of the game

The game has more than 800 natives. The `Context` has functions for the most
useful ones. For an other native, use `CallNative()` with its name. The native
names are in the scripts of the game. When the game does not have the native,
the log of the plugin shows `script: no native <name>`.

> **Note:** A C++ script cannot start a second C++ script inside itself.

## Event flags

An **event flag** remembers that something happened: "the player received the
Pokédex", "the player beat the Gym Leader". The game saves the flags.

The known flags are in `lib/include/core/constant/event_flag.h`.
The game never uses the flags from `EventFlag::kFirstFree` (3026) to
`EventFlag::kLastFree` (3039): 14 flags. Use them for your scripts, with
`HackFlag()` (0 to 13):

```cpp
constexpr EventFlag kMyGiftReceived = core::HackFlag(0);
constexpr EventFlag kMyRivalBeaten = core::HackFlag(1);
```

Use a flag to make sure that a gift happens one time only:

```cpp
if (s.GetFlag(kMyGiftReceived)) {
  s.Talk(u"Take good care of your Pokémon!");
  return;
}
```

## The flags and the variables of the ROM hack

14 flags are not enough for a story. The save data has a reserved part that
the game never uses. The library keeps more flags and variables there, for
your ROM hack only (`savedata::HackSave`):

| What | Numbers | Values | In a script |
|---|---|---|---|
| Flags | 0 to 4095 | true or false | `GetHackFlag(n)`, `SetHackFlag(n)`, `ResetHackFlag(n)` |
| Variables | 0 to 511 | 0 to 65535 | `GetHackVariable(n)`, `SetHackVariable(n, value)` |

Give a name to each number:

```cpp
// The flags of my ROM hack.
constexpr u16 kRivalMet = 0;
constexpr u16 kRivalBeaten = 1;
// The variables of my ROM hack.
constexpr u16 kQuestStep = 0;

void Rival(script::Context& s) {
  s.TalkStart();
  if (!s.GetHackFlag(kRivalMet)) {
    s.Talk(u"Hi! I am your rival.");
    s.SetHackFlag(kRivalMet);
  }
  s.SetHackVariable(kQuestStep, 2);
  s.TalkEnd();
}
```

Outside a script, use `savedata::HackSave::GetFlag()`, `SetFlag()`,
`GetVariable()`, `SetVariable()` and `AddToVariable()`.

- A new game starts with all the flags false and all the variables 0.
- The player saves the game to keep them, like the other data of the game.
- The save data remembers the product that wrote it. With the save data of an
  other ROM hack, the log of the plugin shows
  `save: an other ROM hack wrote this save data`, and
  `HackSave::IsFromOtherHack()` returns true.
- Alpha Sapphire only. In X, the functions do nothing.

## Related pages

- [Write an overworld script](../tutorials/09-write-an-overworld-script.md)
- Examples: `overlay/src/scripts.cc`, `undertow/src/scripts.cc`
