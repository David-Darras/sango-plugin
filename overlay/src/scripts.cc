#include "script/patch/native_script.h"
#include "script/patch/native_table.h"
#include "script/patch/context.h"
#include "overworld/patch/map_character.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/pokemon_param.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/species_data.h"
#include "pokemon/native/movepool.h"
#include "pokemon/constant/species.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/address.h"
#include "renderer/address.h"
#include "battle/patch/setup.h"
#include "battle/patch/battle.h"
#include "battle/native/config.h"
#include "battle/constant/format.h"
#include "battle/constant/trainer.h"
#include "system/native/string.h"
#include "core/utils.h"
#include "script/constant/script.h"
#include "ui/patch/party_select.h"
#include "factory_set.h"

namespace script {
namespace {
using pokemon::SpeciesId;
using pokemon::FormId;
using pokemon::ItemId;
using pokemon::CoreData;

constexpr u32 TRAINER_BTL_FLAG_NO_LOSE = (1 << 2);

using FactoryPokemonSpec = factory::PokemonSpec;

static constexpr u32 kFactoryTrainerCount = 950;

static CoreData s_backup_team[6];
static u8 s_backup_count = 0;
static FactoryPokemonSpec s_player_specs[3];
static FactoryPokemonSpec s_opponent_specs[3];
static bool s_factory_battle_active = false;

static void BackupOriginalTeam() {
  auto& team = savedata::PokemonTeam::GetInstance();
  s_backup_count = team.count;
  for (u32 i = 0; i < team.count; i++) {
    std::memcpy(&s_backup_team[i], team.pokemons[i]->core, sizeof(CoreData));
  }
}

static void RestoreOriginalTeam() {
  if (s_backup_count == 0) return;
  auto& team = savedata::PokemonTeam::GetInstance();
  team.count = s_backup_count;
  for (u32 i = 0; i < s_backup_count; i++) {
    std::memcpy(team.pokemons[i]->core, &s_backup_team[i], sizeof(CoreData));
    team.pokemons[i]->UpdateRuntimeData();
  }
  team.HealAllPokemons();
  s_backup_count = 0;
}

static void FactoryBattleSetupHook(battle::Config& config,
                                   battle::TrainerId& trainer_id) {
  if (!s_factory_battle_active) return;

  config.format = battle::Format::kSingle;
  config.pokemon_teams[0]->count = 3;
  config.pokemon_teams[1]->count = 3;

  for (u32 i = 1; i < 3; i++) {
    *config.pokemon_teams[1]->pokemons[i]->core = *config.pokemon_teams[1]->
        pokemons[0]->core;
    *config.pokemon_teams[1]->pokemons[i]->runtime = *config.pokemon_teams[1]->
        pokemons[0]->runtime;
  }

  for (u32 i = 0; i < 3; i++) {
    factory::ApplySpec(config.pokemon_teams[1]->pokemons[i],
                       s_opponent_specs[i]);
  }
  config.pokemon_teams[1]->HealAllPokemons();

  config.pokemon_teams[0]->HealAllPokemons();

  if (config.trainer_data[1] != nullptr) {
    for (u32 i = 0; i < 4; i++) {
      config.trainer_data[1]->items[i] = ItemId::kNone;
    }
    config.trainer_data[1]->ai_flags =
        AiFlags::kCasual | AiFlags::kCompetitive | AiFlags::kStrategist;
  }
}

static void FillParty(savedata::PokemonTeam& team,
                      const FactoryPokemonSpec* specs, u32 count) {
  team.count = count;
  for (u32 i = 0; i < count; i++) {
    factory::ApplySpec(team.pokemons[i], specs[i]);
  }
  team.HealAllPokemons();
}

static s32 SelectSpec(Context& s, savedata::PokemonTeam& team,
                      const FactoryPokemonSpec* specs, u32 count,
                      const c16* prompt) {
  FillParty(team, specs, count);
  s.Talk(prompt);
  s32 choice = s.SelectPokemon();
  if (choice < 0 || choice >= (s32)count) return -1;
  return choice;
}

static battle::TrainerId GetRandomTrainer(battle::TrainerId previous) {
  while (true) {
    auto candidate = static_cast<battle::TrainerId>(
      1 + core::Utils::GetRandomValue(kFactoryTrainerCount - 1));
    if (candidate != previous) return candidate;
  }
}

static void GenerateDraft(FactoryPokemonSpec* draft, u32 count) {
  SpeciesId species[6];
  ItemId items[6];
  for (u32 i = 0; i < count; i++) {
    species[i] = factory::GetRandomSpecies(species, i);
    draft[i] = factory::BuildSpec(species[i], items, i);
    items[i] = draft[i].item;
  }
}

static battle::BattleSettings s_saved_battle_settings;

static void EnterFactoryRules() {
  auto& battle = battle::Battle::GetInstance();
  s_saved_battle_settings = battle;
  battle.mega_restriction = true;
  battle.unlimited_mega_evolution = false;
  battle.can_use_item = false;
  battle::Setup::GetInstance().on_trainer_battle = FactoryBattleSetupHook;
  s_factory_battle_active = true;
}

static void LeaveFactoryRules() {
  if (!s_factory_battle_active) return;
  static_cast<battle::BattleSettings&>(battle::Battle::GetInstance()) =
      s_saved_battle_settings;
  battle::Setup::GetInstance().on_trainer_battle = nullptr;
  battle::Setup::GetInstance().trainer_id = battle::TrainerId::kNone;
  s_factory_battle_active = false;
}

static bool AskLeave(Context& s) {
  s.ShowMessage(u"Do you want to leave the Battle Factory?");
  bool leave = s.AskYesNo();
  s.CloseMessage();
  return leave;
}

static bool SelectRentalsOneByOne(Context& s, savedata::PokemonTeam& team,
                                  const FactoryPokemonSpec* rentals) {
  u32 remaining[6] = {0, 1, 2, 3, 4, 5};
  u32 remaining_count = 6;

  for (u32 picked = 0; picked < 3; picked++) {
    while (true) {
      FactoryPokemonSpec shown[6];
      for (u32 i = 0; i < remaining_count; i++)
        shown[i] = rentals[remaining[i]];

      c16 prompt[96];
      core::Utils::Format(prompt, u"Choose rental Pokemon %u/3.", picked + 1);
      s32 choice = SelectSpec(s, team, shown, remaining_count, prompt);

      if (choice >= 0) {
        s_player_specs[picked] = shown[choice];
        for (u32 i = choice; i + 1 < remaining_count; i++)
          remaining[i] = remaining[i + 1];
        remaining_count--;
        s.PlaySound(kSoundDecide);
        break;
      }
      if (AskLeave(s)) return false;
    }
  }
  return true;
}

static bool SelectRentals(Context& s, savedata::PokemonTeam& team,
                          const FactoryPokemonSpec* rentals) {
  while (true) {
    FillParty(team, rentals, 6);
    s.Talk(u"Choose the 3 Pokemon you want for your team.");

    u8 order[3];
    switch (s.SelectParty(3, order)) {
      case ui::PartySelect::Status::kSelected:
        for (u32 i = 0; i < 3; i++) s_player_specs[i] = rentals[order[i]];
        s.PlaySound(kSoundDecide);
        return true;
      case ui::PartySelect::Status::kCancelled:
        if (AskLeave(s)) return false;
        break;
      default:
        return SelectRentalsOneByOne(s, team, rentals);
    }
  }
}

void LittlerootGreeter(Context& s) {
  s.TalkStart();
  s.Talk(
      u"Welcome to the Battle Factory!\nHere, only your battling skills matter.");
  s.ShowMessage(u"Would you like to take the rental challenge?");
  if (!s.AskYesNo()) {
    s.Talk(u"Come back when you are ready!");
    s.TalkEnd();
    return;
  }

  auto battle_native = NativeTable::Find("_CallTrainerBattleCore");
  if (battle_native == nullptr) {
    s.Talk(u"Error: Unable to start battle.");
    s.TalkEnd();
    return;
  }

  auto& team = savedata::PokemonTeam::GetInstance();

  BackupOriginalTeam();

  for (u32 i = team.count; i < 6; i++) {
    *team.pokemons[i]->core = *team.pokemons[0]->core;
    *team.pokemons[i]->runtime = *team.pokemons[0]->runtime;
  }

  s.Talk(
      u"Your personal Pokemon are kept safe.\nChoose 3 of the 6 rental Pokemon.");

  FactoryPokemonSpec rentals[6];
  GenerateDraft(rentals, 6);

  if (!SelectRentals(s, team, rentals)) {
    RestoreOriginalTeam();
    s.Talk(
        u"Here is your original team. See you next time at the Battle Factory!");
    s.TalkEnd();
    return;
  }

  FillParty(team, s_player_specs, 3);
  s.Talk(u"Your rental team is ready! The tournament begins!");

  EnterFactoryRules();

  u32 streak = 0;
  battle::TrainerId trainer = battle::TrainerId::kNone;

  while (true) {
    FactoryPokemonSpec opp_draft[6];
    GenerateDraft(opp_draft, 6);

    bool opp_picked[6] = {false};
    for (u32 i = 0; i < 3; i++) {
      u32 pick;
      do {
        pick = core::Utils::GetRandomValue(6);
      } while (opp_picked[pick]);
      opp_picked[pick] = true;
      s_opponent_specs[i] = opp_draft[pick];
    }

    trainer = GetRandomTrainer(trainer);
    battle::Setup::GetInstance().trainer_id = trainer;

    c16 prompt[128];
    core::Utils::Format(prompt, u"Battle #%u! Your opponent steps forward!",
                        streak + 1);
    s.Talk(prompt);

    s.Call(battle_native,
           0,
           (s32)trainer,
           0,
           TRAINER_BTL_FLAG_NO_LOSE,
           0, 0, 0);

    s.Yield();

    auto layout_fade = NativeTable::Find("LayoutFadeRequestIn");
    if (layout_fade != nullptr) {
      s.Call(layout_fade);
    }
    auto fade_native = NativeTable::Find("_FadeRequestIn");
    if (fade_native != nullptr) {
      s.Call(fade_native, 2, 30);
    }
    ((void (*)(u32, u32))renderer::address::kFadeRequestIn)(2, 30);
    s.Wait(30);

    if (!s.WonLastBattle()) {
      s.Talk(u"All your Pokemon fainted!");
      c16 end_msg[128];
      core::Utils::Format(end_msg, u"Your streak ended at %u consecutive wins.",
                          streak);
      s.Talk(end_msg);
      break;
    }

    streak++;
    c16 win_msg[128];
    core::Utils::Format(win_msg, u"Congratulations on winning Battle #%u!",
                        streak);
    s.Talk(win_msg);

    team.HealAllPokemons();

    s.ShowMessage(
        u"Would you like to swap one of your Pokemon with the opponent's?");
    bool swap = s.AskYesNo();
    s.CloseMessage();

    bool swapped = false;
    if (swap) {
      s32 mine = SelectSpec(s, team, s_player_specs, 3,
                            u"Choose the Pokemon you want to trade away.");
      s32 theirs = -1;
      if (mine >= 0) {
        theirs = SelectSpec(s, team, s_opponent_specs, 3,
                            u"Choose the opponent's Pokemon you want to take.");
      }
      if (mine >= 0 && theirs >= 0) {
        FactoryPokemonSpec temp = s_player_specs[mine];
        s_player_specs[mine] = s_opponent_specs[theirs];
        s_opponent_specs[theirs] = temp;
        swapped = true;
      }
      FillParty(team, s_player_specs, 3);
    }
    s.Talk(swapped
             ? u"Swap completed successfully!"
             : u"You kept your current team.");

    s.ShowMessage(u"Would you like to continue to the next battle?");
    bool next = s.AskYesNo();
    s.CloseMessage();
    if (!next) {
      c16 quit_msg[128];
      core::Utils::Format(quit_msg,
                          u"You retired with a great streak of %u wins!",
                          streak);
      s.Talk(quit_msg);
      break;
    }
  }

  LeaveFactoryRules();

  RestoreOriginalTeam();
  s.Talk(
      u"Here is your original team. See you next time at the Battle Factory!");
  s.TalkEnd();
}

#ifdef GAME_XY
void KujiraGreeter(Context& s) {
  s.TalkStart();
  s.Talk(u"This is a new script generated with ZettaD's Kujira plugin.");
  s.Talk(u"I'm going to turn all your Pokémon into shinies.");
  s.TalkEnd();
  s.PlayJingle(kJingleItem);
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    auto& pkm = team.pokemons[i];
    auto& core = *pkm->core;
    pkm->accessor->Decrypt();
    if (pokemon::Utils::IsShiny(core.id, core.shiny_id)) {
      pokemon::Utils::ConvertToNormal(core.id, &core.shiny_id);
    } else {
      pokemon::Utils::ConvertToShiny(core.id, &core.shiny_id);
    }
    pkm->accessor->Encrypt();
  }
}
#endif
}

void Install() {
#ifdef GAME_XY
  NativeScript::Register(ScriptId::kKujiraGreeter, KujiraGreeter);

  overworld::MapCharacterRequest greeter;
  greeter.map_id = static_cast<MapId>(264);
  greeter.model_id = ModelId::kTeamFlareAdminMale;
  greeter.script_id = ScriptId::kKujiraGreeter;
  greeter.tile_x = 593;
  greeter.tile_z = 492;
  greeter.facing = overworld::Facing::kRight;
  overworld::MapCharacter::Add(greeter);
#else
  NativeScript::Register(ScriptId::kLittlerootGreeter,
                         LittlerootGreeter);

  overworld::MapCharacterRequest greeter;
  greeter.map_id = MapId::kBattleResortBA_2;
  greeter.model_id = ModelId::kResearcherMale;
  greeter.script_id = ScriptId::kLittlerootGreeter;
  greeter.tile_x = 47;
  greeter.tile_z = 77;
  greeter.height = 6;
  greeter.movement_id = 17;
  greeter.facing = overworld::Facing::kDown;
  overworld::MapCharacter::Add(greeter);
#endif
}
}