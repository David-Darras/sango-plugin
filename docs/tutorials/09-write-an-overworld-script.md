# Tutorial 9: Write an overworld script

In this tutorial, you add a new character to a map. When the player talks to
the character, the character asks a question and gives 5 Rare Candies.
The gift happens one time only.

**Time:** 25 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md) and the overlay
(`sango_plugin-release.3gx`) to find a position.
**Read first:** [Scripts](../concepts/scripts.md).

## Step 1: Find a position

You need the map id and the tile position of the new character.

1. Install the overlay (`sango_plugin-release.3gx`) and start the game.
2. Walk to the place of the new character. Use an empty tile.
3. Open the menu: **Overworld > Map Id**. Write down the number.
4. Open the menu: **Player**. Write down **Tile X**, **Tile Y** and **Tile Z**.
   **Tile Y** is the height.
5. Find the name of the map id in `lib/include/overworld/constant/map.h`.

## Step 2: Write the script

Make the file `myhack/src/my_scripts.cc`. Replace the map and the position with
your values:

```cpp
/**
 * @file my_scripts.cc
 * @brief The C++ scripts of my ROM hack.
 */

#include "core/constant/event_flag.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/patch/map_character.h"
#include "pokemon/constant/item.h"
#include "script/patch/native_script.h"

namespace myhack {
namespace {

// The id of the script. The ids 30500 to 59999 are free.
constexpr ScriptId kCandyGiver = static_cast<ScriptId>(31000);

// A flag that the game never uses. It remembers that the gift happened.
constexpr EventFlag kCandyReceived =
    static_cast<EventFlag>(static_cast<u16>(EventFlag::kFirstFree) + 1);

// The script. The game runs it when the player talks to the character.
void CandyGiver(script::Context& s) {
  s.TalkStart();

  if (s.GetFlag(kCandyReceived)) {
    s.Talk(u"Use the candies well!");
    s.TalkEnd();
    return;
  }

  s.Talk(u"Hello, Trainer!\nDo you like candies?");
  if (!s.AskYesNo()) {
    s.Talk(u"Oh... Come back if you change your mind.");
    s.TalkEnd();
    return;
  }

  if (!s.CanGiveItem(ItemId::kRareCandy, 5)) {
    s.Talk(u"Your Bag is full!");
    s.TalkEnd();
    return;
  }

  s.GiveItem(ItemId::kRareCandy, 5);
  s.PlayJingle(script::kJingleItem);
  s.SetFlag(kCandyReceived);
  s.Talk(u"Here are 5 Rare Candies!");
  s.TalkEnd();
}

} // namespace

void InstallScripts() {
  // 1. Link the script id to the C++ function.
  script::NativeScript::Register(kCandyGiver, CandyGiver);

  // 2. Put a character on the map, with this script.
  overworld::MapCharacterRequest request;
  request.map_id = MapId::kLittlerootTown;      // Your map.
  request.model_id = ModelId::kResearcherMale;  // The look of the character.
  request.script_id = kCandyGiver;
  request.tile_x = 47;                          // Your Tile X.
  request.tile_z = 77;                          // Your Tile Z.
  request.height = 6;                           // Your Tile Y.
  request.facing = overworld::Facing::kDown;
  overworld::MapCharacter::Add(request);
}

} // namespace myhack
```

## Step 3: Install the script

In `myhack/src/entrypoint.cc`, declare `myhack::InstallScripts()` and call it
in `Initialize()`, after `plugin::InitializeEngine()`:

```cpp
namespace myhack {
void InstallScripts();
}

void Initialize() {
  plugin::InitializeEngine();
  InstallCallbacks();
  myhack::InstallScripts();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), LoadRootPage);
  plugin::Start(EveryFrame);
}
```

## Step 4: Build and test

```bash
make myhack
```

1. Go to the map. The new character stands at your position.
2. Talk to the character. Answer **Yes**: you get 5 Rare Candies.
3. Talk to the character again: the character says "Use the candies well!".

> **Note:** The flag is in the save data. Save the game to keep it.

## The options of a character

| Member of `MapCharacterRequest` | Meaning |
|---|---|
| `map_id` | The map. |
| `model_id` | The model (`lib/include/overworld/constant/model.h`). |
| `script_id` | The script that runs when the player talks to the character. |
| `tile_x`, `tile_z`, `height` | The position. |
| `facing` | The direction: `kUp`, `kDown`, `kLeft`, `kRight`. |
| `movement_id` | The movement code of the character. 0 is the default value. |
| `hide_when_flag_set` | An event flag. When it is set, the character is not there. 0 = always there. |
| `is_everywhere` | `true`: the character is on all the maps. |
| `is_enabled` | A pointer to a `bool` of your product. When the `bool` is `false`, the character is not there. |

You can add 16 characters at most.

## Remove the characters of a map

`overworld::MapCharacter::Empty(map_id)` removes the characters of the game
from a map. Then only your characters are on this map.
See `undertow/src/scripts.cc`.

## Examples in the project

| File | Script |
|---|---|
| `overlay/src/scripts.cc` | A greeter, and a complete Battle Factory with team selection. |
| `undertow/src/scripts.cc` | A boss that gives a starter Pokémon. |

## What you learned

- `NativeScript::Register` links a script id to a C++ function.
- `MapCharacter::Add` puts a character with a script on a map.
- `script::Context` has the functions of the script: messages, items, flags...
- A free event flag makes a gift happen one time only.

**Next:** [Tutorial 10: Replace models](10-replace-models.md)
