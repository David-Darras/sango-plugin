# Tutorial 8: Change the trainer teams

In this tutorial, you give a new team to the first trainer of Route 102:
a Pichu and a Riolu with competitive moves and EVs.

**Time:** 20 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

1. The player starts a battle against a trainer.
2. The game prepares the battle: the trainer, the Pokémon, the place.
3. `battle::Setup` calls your callback `on_trainer_battle`.
4. Your callback changes the battle with `battle::TrainerTeams`.

`battle::TrainerTeam` describes a team: the format, the place and up to six
Pokémon (`battle::TrainerOpponent`).

## Step 1: Write the team

Make the file `myhack/src/my_trainers.cc`:

```cpp
/**
 * @file my_trainers.cc
 * @brief The trainer teams of my ROM hack.
 */

#include "battle/constant/format.h"
#include "battle/constant/trainer.h"
#include "battle/native/config.h"
#include "battle/patch/setup.h"
#include "battle/patch/trainer_team.h"

namespace myhack {
namespace {

// Each Pokemon: species, held item, ability, nature, shiny,
// EVs (HP, Atk, Def, Sp. Atk, Sp. Def, Speed), four moves,
// then (optional) form, nickname, level.
const battle::TrainerTeam kRoute102Kid1Team(
    2, Format::kSingle,
    {
        {SpeciesId::kPichu, ItemId::kOranBerry, AbilityId::kStatic,
         Nature::kTimid, false,
         0, 0, 0, 252, 4, 252,
         MoveId::kThunderShock, MoveId::kCharm, MoveId::kSweetKiss,
         MoveId::kNone},
        {SpeciesId::kRiolu, ItemId::kNone, AbilityId::kInnerFocus,
         Nature::kAdamant, false,
         0, 252, 4, 0, 0, 252,
         MoveId::kQuickAttack, MoveId::kEndure, MoveId::kCounter,
         MoveId::kNone,
         FormId::kNormal, nullptr, 8},
    });

// The list of the new teams: one line for each trainer.
const battle::TrainerTeamEntry kTeams[] = {
    {TrainerId::kRoute102Kid1, &kRoute102Kid1Team},
};

// The game calls this function before each trainer battle.
void OnTrainerBattle(battle::Config& config, TrainerId& trainer_id) {
  // If the trainer is in kTeams, its team replaces the team of the game.
  battle::TrainerTeams::Apply(config, trainer_id);
}

} // namespace

void InstallTrainers() {
  battle::TrainerTeams::SetTable(kTeams, SIZE(kTeams));
  battle::Setup::GetInstance().on_trainer_battle = OnTrainerBattle;
}

} // namespace myhack
```

## Step 2: Install the callback

In `myhack/src/entrypoint.cc`, declare `myhack::InstallTrainers()` and call it
in `InstallCallbacks()`:

```cpp
namespace myhack {
void InstallEncounters();
void InstallTrainers();
}

void InstallCallbacks() {
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
  myhack::InstallEncounters();
  myhack::InstallTrainers();
}
```

## Step 3: Build and test

```bash
make myhack
```

Battle the first trainer of Route 102. The trainer has a Pichu and a Riolu.

## The levels

The new Pokémon copy the level of the first Pokémon of the game team.
To set a level, give the last value of the Pokémon (`8` for Riolu above).

## The place of the battle

The short constructor (`opponent_count, format, {pokemon...}`) keeps the place
of the game. The long constructor also sets the place:

```cpp
const battle::TrainerTeam kBossTeam(
    3, Format::kDouble,
    BackgroundId::kAquaBoss, GroundId::kAquaBoss, PlatformId::kWater,
    EncounterAnimationId::kKyogre, battle::Weather::kHeavyRain,
    {
        // The Pokémon...
    });
```

| Value | Meaning |
|---|---|
| `Format` | `kSingle`, `kDouble`, `kTriple`, `kRotation`, `kHorde`. |
| `BackgroundId`, `GroundId`, `PlatformId` | The look of the battlefield. |
| `EncounterAnimationId` | The animation at the start of the battle. |
| `battle::Weather` | The weather at the start. `Weather::kInvalid` keeps the normal weather. |

The values are in `lib/include/battle/constant/`.

## Find the id of a trainer

- The trainer ids are in `lib/include/battle/constant/trainer.h`.
- Not all the trainers have a name in this file. You can use a number:
  `static_cast<TrainerId>(42)`.
- In the callback, write the id to the log to find it:

  ```cpp
  ui::LogApplication::Print(u"Trainer %u", static_cast<u32>(trainer_id));
  ```

## Go further

`kaizo/src/kaizo_trainer.cc` changes much more:

- the level of the trainer Pokémon (it follows the level of the player),
- the AI of the trainers,
- special rules for some trainers (inverse battle, Metronome only...).

## What you learned

- `battle::TrainerTeam` describes a team.
- `battle::TrainerTeams::SetTable` registers the teams.
- `on_trainer_battle` and `TrainerTeams::Apply` replace the team before the battle.

**Next:** [Tutorial 9: Write an overworld script](09-write-an-overworld-script.md)
