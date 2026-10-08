# Tutorial 5: Add an ability

In this tutorial, you add a new ability: **Quick Start**.
When the Pokémon enters a battle, its Speed increases by one stage.

**Time:** 15 minutes.
**You need:** [Tutorial 4](04-add-a-move.md).
**Read first:** [The battle engine](../concepts/battle-engine.md).

## How a new ability works

Alpha Sapphire has 191 abilities (ids 1 to 191). An ability id is one byte,
so a new ability has an id from 192 to 255.

A new ability has:

- a name and a description,
- reactions: the functions that the battle engine calls at some moments.

## Step 1: Choose an id

The overlay uses the ids 248 to 255. Use a different id, for example 230.

## Step 2: Write the ability

Make the file `myhack/src/my_abilities.cc`:

```cpp
/**
 * @file my_abilities.cc
 * @brief The new abilities of my ROM hack.
 */

#include "battle/constant/moment_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/situation.h"
#include "battle/patch/game_extension.h"

namespace myhack {

/// The id of Quick Start.
constexpr AbilityId kAbilityQuickStart = static_cast<AbilityId>(230);

namespace {

// The reaction: the battle engine calls it when a Pokemon enters the battle.
void QuickStartReaction(battle::Listener* self, battle::Controller* controller,
                        battle::UID owner, s32* local_state) {
  // The moment is for all the Pokemon. Continue only for the owner.
  if (battle::Situation::Get(battle::SituationKey::kPokemonId) !=
      owner.value) {
    return;
  }

  // Ask for a stat change: +1 Speed for the owner.
  auto* mutation = static_cast<battle::AdjustStatStageMutation*>(
      controller->Create(battle::MutationKind::kAdjustStatStage, owner));
  mutation->show_ability_banner = true;  // Shows "Quick Start" on screen.
  mutation->target_count = 1;
  mutation->target_ids[0] = owner;
  mutation->stage_kind = battle::StatStageEffectKind::kSpeed;
  mutation->stage_delta = 1;
  controller->Apply(mutation);
}

const battle::ReactionTable kQuickStartReactions[] = {
    {battle::MomentKind::kPokemonEntered, QuickStartReaction},
};

} // namespace

void RegisterAbilities() {
  battle::GameExtension::AddAbility(
      {kAbilityQuickStart, u"Quick Start",
       u"Raises the Speed of the Pokémon\nwhen it enters a battle.",
       kQuickStartReactions, SIZE(kQuickStartReactions)});
}

} // namespace myhack
```

## Step 3: Register the ability at the start

In `myhack/src/entrypoint.cc`:

```cpp
#include "myhack/my_moves.h"

namespace myhack {
void RegisterAbilities();
}

void Initialize() {
  plugin::InitializeEngine();
  InstallCallbacks();
  myhack::RegisterMoves();
  myhack::RegisterAbilities();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), LoadRootPage);
  plugin::Start(EveryFrame);
}
```

## Step 4: Give the ability to a Pokémon

Add this action to `myhack/src/my_pages.cc`, like in [Tutorial 4](04-add-a-move.md):

```cpp
static void GiveQuickStart(void* args) {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;

  savedata::PokemonParam* pokemon = team.pokemons[0];
  pokemon->accessor->Decrypt();
  pokemon->core->ability = static_cast<AbilityId>(230);
  pokemon->accessor->Encrypt();
}
```

```cpp
app.Add("Heal the party", HealParty)
   .Add("Teach Thunderclap", TeachThunderclap)
   .Add("Give Quick Start", GiveQuickStart);
```

## Step 5: Build and test

```bash
make myhack
```

1. In the overworld, open the menu: **My options > Party > Give Quick Start**.
2. Open the summary of the first Pokémon: the ability is **Quick Start**.
3. Start a battle. The banner "Quick Start" shows, and the Speed increases.

## Useful moments

The moments are in `lib/include/battle/constant/moment_kind.h`.

| Moment | When |
|---|---|
| `kPokemonEntered` | A Pokémon enters the battle. |
| `kAfterAbilityChange` | A Pokémon gets a new ability (for example with Skill Swap). |
| `kMoveExecutionStart` | A move starts. |
| `kDamageSequenceEndRealHit` | A move did damage. |

> **Tip:** An ability that acts on entry usually reacts to `kPokemonEntered`
> **and** `kAfterAbilityChange`. See the Surge abilities in
> `overlay/src/custom_abilities.cc`.

## Examples in the project

`overlay/src/custom_abilities.cc` contains complete examples:

| Ability | What it shows |
|---|---|
| Toxic Drizzle | Weather + status condition. |
| Radioactive Drizzle | HP change, form change, type change. |
| Reality Warp | The Pokémon uses several moves. |
| Electric Surge, Grassy Surge, Misty Surge | Terrain. |
| Beast Boost | A stat change after a knock out, with the situation values. |

## What you learned

- A new ability has an id from 192 to 255 and a `battle::AbilitySpec`.
- A reaction checks the situation, then asks for a mutation.
- `GameExtension::AddAbility` registers the ability.

**Next:** [Tutorial 6: Make a move animation](06-make-a-move-animation.md)
