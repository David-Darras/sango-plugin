# Tutorial 4: Add a move

In this tutorial, you add a new move: **Thunderclap**.

- Type: Electric. Category: Special.
- Power 70, accuracy 100, 5 PP, priority +1.
- When the Pokémon uses it, rain starts.
- It uses the animation of Thunder Shock.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md) and [Tutorial 2](02-add-a-menu-page.md).
**Read first:** [The battle engine](../concepts/battle-engine.md).

## How a new move works

The game has 621 moves (ids 1 to 621). A new move has an id above 621.
`battle::GameExtension` makes the game accept this id:

- It gives the name and the description of the move to the menus.
- It loads the data of Pound (move 1), then your function changes this data.
- It gives the reactions of the move to the battle engine.
- It selects the animation of the move.

## Step 1: Choose an id

Choose an id above 621 that no other move uses.
The overlay uses 863 and 864. Use a different id, for example 900.

## Step 2: Write the move

Make the file `myhack/include/myhack/my_moves.h`. Other files include it to
know the id of the move:

```cpp
#pragma once

#include "common.h"
#include "pokemon/constant/move.h"

namespace myhack {
/// The id of Thunderclap.
constexpr MoveId kMoveThunderclap = static_cast<MoveId>(900);

/// Adds the new moves of my ROM hack to the game.
void RegisterMoves();
} // namespace myhack
```

Make the file `myhack/src/my_moves.cc`:

```cpp
/**
 * @file my_moves.cc
 * @brief The new moves of my ROM hack.
 */

#include "myhack/my_moves.h"

#include "battle/constant/moment_kind.h"
#include "battle/constant/weather.h"
#include "battle/native/controller.h"
#include "battle/patch/game_extension.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/type.h"
#include "pokemon/native/move_data.h"

namespace myhack {
namespace {

// 1. The data of the move. The game starts from the data of Pound.
void PatchThunderclapData(pokemon::MoveData& move) {
  move.type = TypeId::kElectric;
  move.category = 0;          // 0 = damage only.
  move.damage_category = 2;   // 2 = special (1 = physical, 0 = status).
  move.power = 70;
  move.accuracy = 100;        // 101 = the move never misses.
  move.base_pp = 5;
  move.priority = 1;
}

// 2. The animation of the move: the animation of Thunder Shock.
void UseThunderShockAnimation(u32& id, bool& is_move) {
  id = static_cast<u32>(MoveId::kThunderShock);
  is_move = true;
}

// 3. The reaction: rain starts when the move starts.
void ThunderclapRainReaction(battle::Listener* self,
                             battle::Controller* controller,
                             battle::UID owner, s32* local_state) {
  // false: the rain stops after 5 turns (8 turns with a Damp Rock).
  controller->SetWeather(owner, battle::Weather::kRain, ItemId::kDampRock,
                         false);
}

const battle::ReactionTable kThunderclapReactions[] = {
    {battle::MomentKind::kMoveExecutionStart, ThunderclapRainReaction},
};

} // namespace

// 4. Register the move.
void RegisterMoves() {
  battle::GameExtension::AddMove(
      {kMoveThunderclap, u"Thunderclap",
       u"A fast electric attack.\nIt calls the rain.",
       PatchThunderclapData, UseThunderShockAnimation,
       kThunderclapReactions, SIZE(kThunderclapReactions)});
}

} // namespace myhack
```

The text `u"..."` is a UTF-16 text: the game uses this format.
`\n` starts a new line in the description.

## Step 3: Register the move at the start

In `myhack/src/entrypoint.cc`, include the header and call the function in `Initialize()`:

```cpp
#include "myhack/my_moves.h"

void Initialize() {
  plugin::InitializeEngine();
  InstallCallbacks();
  myhack::RegisterMoves();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), LoadRootPage);
  plugin::Start(EveryFrame);
}
```

## Step 4: Give the move to a Pokémon

The new move is not in any learnset. For a test, add a menu action that teaches
it to the first Pokémon of the party.

In `myhack/src/my_pages.cc` (from [Tutorial 2](02-add-a-menu-page.md)):

```cpp
#include "myhack/my_moves.h"

static void TeachThunderclap(void* args) {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;

  savedata::PokemonParam* pokemon = team.pokemons[0];
  pokemon->accessor->Decrypt();
  pokemon->core->moves[0] = myhack::kMoveThunderclap;  // The first move slot.
  pokemon->core->pp[0] = 5;
  pokemon->core->pp_up_count[0] = 0;
  pokemon->accessor->Encrypt();
}
```

Add it to `LoadPartyPage`:

```cpp
app.Add("Heal the party", HealParty)
   .Add("Teach Thunderclap", TeachThunderclap);
```

## Step 5: Build and test

```bash
make myhack
```

1. In the overworld, open the menu: **My options > Party > Teach Thunderclap**.
2. Open the summary of the first Pokémon: the first move is **Thunderclap**.
3. Start a battle and use Thunderclap. The animation of Thunder Shock plays and rain starts.

## The data of a move

These are the members of `pokemon::MoveData` that you change most:

| Member | Values |
|---|---|
| `type` | `TypeId::kNormal`, `TypeId::kFire`... |
| `damage_category` | 0 = status, 1 = physical, 2 = special. |
| `category` | 0 = damage, 1 = status condition only, 2 = stat change only, 3 = recovery, 4 = damage + status condition, 6 = damage + stat change of the target, 7 = damage + stat change of the user, 8 = HP drain, 9 = one-hit KO. |
| `power` | The power. 0 for a status move. |
| `accuracy` | 1 to 100. 101 = the move never misses. |
| `base_pp` | The PP. |
| `priority` | -7 to +5. 0 for most moves. |
| `effect_id` | The status condition (`StatusCondition::kBurn`...). |
| `effect_rate` | The chance of the status condition, in percent. |
| `flinch_rate` | The chance of a flinch, in percent. |
| `crit_stage` | The critical hit stage. |
| `recoil`, `drain` | The recoil or the HP drain, in percent. |
| `target` | 0 = one other Pokémon (selected), 3 = one opponent, 4 = all the other Pokémon, 5 = all the opponents, 7 = the user. |
| `stat_id`, `stat_stages`, `stat_rate` | Up to three stat changes: the stat, the number of stages, the chance in percent. |

The values come from the data of the game. The page **Pokemon > Move Data** of
the overlay shows the data of all the moves: use it to copy the values of a
similar move.

## What you learned

- A new move has an id above 621 and a `battle::MoveSpec`.
- `patch_data` changes the data of Pound into the data of your move.
- A reaction changes the battle at a moment.
- `GameExtension::AddMove` registers the move.

**Next:** [Tutorial 5: Add an ability](05-add-an-ability.md)
