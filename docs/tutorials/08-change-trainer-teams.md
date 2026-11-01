# Tutorial 8: Change the trainer teams

In this tutorial, you give a new team to the first trainer of Route 102:
a Pichu and a Riolu with competitive moves and EVs.

**Time:** 15 minutes.
**You need:** [Tutorial 1](01-create-your-rom-hack.md).

## How it works

1. You describe a team with `battle::TrainerTeam` and `battle::TrainerOpponent`.
2. You link the team to a trainer with `battle::TrainerTeams::Add()`.
3. Before a battle against this trainer, the library replaces the team of the
   game with your team.

## Step 1: Write the team

Make the file `myhack/src/my_trainers.cc`:

```cpp
/**
 * @file my_trainers.cc
 * @brief The trainer teams of my ROM hack.
 */

#include "sango.h"

namespace myhack {
namespace {

// A team: the format of the battle, then each Pokémon. Give the species,
// then the values that you want to change. The other values come from the
// game.
const battle::TrainerTeam kRoute102Kid1Team =
    battle::TrainerTeam(Format::kSingle)
        .Add(battle::TrainerOpponent(SpeciesId::kPichu)
                 .WithLevel(7)
                 .WithItem(ItemId::kOranBerry)
                 .WithAbility(AbilityId::kStatic)
                 .WithNature(Nature::kTimid)
                 .WithEvs(0, 0, 0, 252, 4, 252)
                 .WithMoves(MoveId::kThunderShock, MoveId::kCharm,
                            MoveId::kSweetKiss))
        .Add(battle::TrainerOpponent(SpeciesId::kRiolu)
                 .WithLevel(8)
                 .WithAbility(AbilityId::kInnerFocus)
                 .WithNature(Nature::kAdamant)
                 .WithIvs(31)
                 .WithEvs(0, 252, 4, 0, 0, 252)
                 .WithMoves(MoveId::kQuickAttack, MoveId::kEndure,
                            MoveId::kCounter));

} // namespace

void InstallTrainers() {
  battle::TrainerTeams::Add(TrainerId::kRoute102Kid1, &kRoute102Kid1Team);
}

} // namespace myhack
```

## Step 2: Install the teams

In `myhack/src/entrypoint.cc`, declare `myhack::InstallTrainers()` and call it
in `SetUpHack()`:

```cpp
namespace myhack {
void InstallTrainers();
}

void SetUpHack() {
  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_1;
  myhack::InstallTrainers();
}
```

Keep the lines of the other tutorials that you did (for example
`myhack::InstallEncounters()` of [Tutorial 7](07-change-wild-pokemon.md)).

## Step 3: Build and test

```bash
make myhack
```

Battle the first trainer of Route 102. The trainer has a Pichu and a Riolu.

## The values of a Pokémon

| Function | Value | Without the function |
|---|---|---|
| `WithLevel(level)` | The level (1 to 100). | The level of the first Pokémon of the game team. |
| `WithItem(item)` | The held item. | No item. |
| `WithAbility(ability)` | The ability. | The ability of the species. |
| `WithNature(nature)` | The nature. | Hardy. |
| `WithShiny()` | The Pokémon is shiny. | Not shiny. |
| `WithEvs(hp, atk, def, spa, spd, spe)` | The six EVs, in the order of the summary. | 0. |
| `WithIvs(value)` | The six IVs (0 to 31). | The IVs of the game. |
| `WithMoves(m1, m2, m3, m4)` | The moves. A move that you do not give is empty. | No moves. |
| `WithForm(form)` | The form. | The normal form. |
| `WithNickname(u"Name")` | The nickname. | The name of the species. |

## The place of the battle

A team keeps the place of the game. To change it, use `WithPlace()` and
`WithWeather()` on the team:

```cpp
const battle::TrainerTeam kBossTeam =
    battle::TrainerTeam(Format::kDouble)
        .WithPlace(BackgroundId::kAquaBoss, GroundId::kAquaBoss,
                   PlatformId::kWater, EncounterAnimationId::kKyogre)
        .WithWeather(battle::Weather::kHeavyRain)
        .Add(battle::TrainerOpponent(SpeciesId::kSharpedo).WithLevel(30))
        .Add(battle::TrainerOpponent(SpeciesId::kMightyena).WithLevel(30));
```

| Value | Meaning |
|---|---|
| `Format` | `kSingle`, `kDouble`, `kTriple`, `kRotation`, `kHorde`. |
| `BackgroundId`, `GroundId`, `PlatformId` | The look of the battlefield. |
| `EncounterAnimationId` | The animation at the start of the battle. |
| `battle::Weather` | The weather at the start. `Weather::kInvalid` keeps the normal weather. |

The values are in `lib/include/battle/constant/`.

## The trainer

The team can also change the trainer:

```cpp
const battle::TrainerTeam kRivalTeam =
    battle::TrainerTeam(Format::kSingle)
        .WithName(u"Rival")
        .WithAi(AiFlags::kCasual | AiFlags::kCompetitive |
                AiFlags::kStrategist)
        .WithItems(ItemId::kHyperPotion, ItemId::kHyperPotion)
        .WithMoney(40)
        .Add(battle::TrainerOpponent(SpeciesId::kRiolu).WithLevel(8));
```

| Function | What it changes |
|---|---|
| `WithName(u"Name")` | The name of the trainer in the battle. The library translates it (see [Tutorial 17](17-translate-your-rom-hack.md)). |
| `WithAi(flags)` | The AI: `kCasual`, `kCompetitive`, `kStrategist`. Combine them with `\|`. |
| `WithItems(item1, ...)` | The items that the trainer uses in the battle (4 at most). |
| `WithMoney(value)` | The prize money: the game multiplies it by the level of the last Pokémon. |

## A new trainer on a map

`overworld::MapCharacter` can put a trainer of the game on any map. The
trainer sees the player, talks and starts the battle, like the trainers of the
game. After a win, it battles no more. Add this to `InstallTrainers()`:

```cpp
overworld::MapCharacterRequest request;
request.map_id = MapId::kRoute101;
request.model_id = ModelId::kYoungster;
request.trainer_id = TrainerId::kRoute102Kid1;  // Its team: kRoute102Kid1Team.
request.sight = 4;                               // It sees 4 tiles.
request.tile_x = 10;
request.tile_z = 12;
request.facing = overworld::Facing::kLeft;
overworld::MapCharacter::Add(request);
```

The trainer says the texts of this trainer of the game. Use a trainer that the
player cannot meet elsewhere, or remove the trainer of the game from its map
(`MapCharacter::Empty()`). The overlay shows the sight of the trainers:
**Overworld > World Overlay**.

## Many teams

`TrainerTeams::Add()` keeps 256 teams. For a long list, use a table and
`TrainerTeams::SetTable()`:

```cpp
const battle::TrainerTeamEntry kTeams[] = {
    {TrainerId::kRoute102Kid1, &kRoute102Kid1Team},
    {TrainerId::kRoute103Brendan1, &kRivalTeam},
};

// In InstallTrainers(), in the place of TrainerTeams::Add():
battle::TrainerTeams::SetTable(kTeams, SIZE(kTeams));
```

## Find the id of a trainer

- The trainer ids are in `lib/include/battle/constant/trainer.h`.
- Not all the trainers have a name in this file. You can use a number:
  `static_cast<TrainerId>(42)`.

## Go further: the callback

`battle::Setup::GetInstance().on_trainer_battle` calls your function before
each trainer battle. It can change all the settings of the battle.

> **Note:** When your product sets this callback, the library does not apply
> the teams of `TrainerTeams` itself. Call `battle::TrainerTeams::Apply()` in
> your callback.

```cpp
void OnTrainerBattle(battle::Config& config, TrainerId& trainer_id) {
  // Writes the id of the trainer to the log (All the pages > Log).
  ui::Log::Print(u"Trainer %u", static_cast<u32>(trainer_id));
  battle::TrainerTeams::Apply(config, trainer_id);
}

// In InstallTrainers():
battle::Setup::GetInstance().on_trainer_battle = OnTrainerBattle;
```

`kaizo/src/kaizo_trainer.cc` changes much more:

- the level of the trainer Pokémon (it follows the level of the player),
- the AI of the trainers,
- special rules for some trainers (inverse battle, Metronome only...).

## What you learned

- `battle::TrainerOpponent` and the With functions describe a Pokémon.
- `battle::TrainerTeam` and `Add()` describe a team.
- `battle::TrainerTeams::Add()` links a team to a trainer.

**Next:** [Tutorial 9: Write an overworld script](09-write-an-overworld-script.md)
