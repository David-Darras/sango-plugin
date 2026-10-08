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
are free for C++ scripts. Define your ids in your product:

```cpp
constexpr ScriptId kMyGreeter = static_cast<ScriptId>(31000);
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

You can add 16 characters at most.

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
| `GetVariable(id)`, `SetVariable(id, value)` | Reads and changes a script variable. |
| `GetMoney()`, `AddMoney(amount)`, `SubMoney(amount)` | Reads and changes the money. |
| `PlaySound(id)`, `PlayJingle(id)` | Plays a sound or a short music. |
| `Walk(object, direction, count)`, `Face(object, direction)` | Moves a character. |
| `Wait(frames)` | Waits some frames. |
| `CallScript(id)` | Runs a game script. |

The full list is in `lib/include/script/patch/context.h`.

> **Note:** A C++ script cannot start a second C++ script inside itself.

## Event flags

An **event flag** remembers that something happened: "the player received the
Pokédex", "the player beat the Gym Leader". The game saves the flags.

The known flags are in `lib/include/core/constant/event_flag.h`.
The game never uses the flags from `EventFlag::kFirstFree` (3026) to
`EventFlag::kLastFree` (3039). Use them for your scripts:

```cpp
constexpr EventFlag kMyGiftReceived =
    static_cast<EventFlag>(static_cast<u16>(EventFlag::kFirstFree) + 1);
```

Use a flag to make sure that a gift happens one time only:

```cpp
if (s.GetFlag(EventFlag::kStarterGiven)) {
  s.Talk(u"Take good care of your Pokémon!");
  return;
}
```

## Related pages

- [Write an overworld script](../tutorials/09-write-an-overworld-script.md)
- Examples: `overlay/src/scripts.cc`, `undertow/src/scripts.cc`
