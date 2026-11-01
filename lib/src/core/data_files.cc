/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file data_files.cc
 * @brief A ROM hack without code: text files that change the game.
 *
 * The declarations are in core/data_files.h. core::maker reads the files
 * (core/maker_reader.h). This file gives the parts to the game, and runs the
 * scripts of story.txt.
 */

#include "core/data_files.h"

#include <cstdarg>
#include <cstdio>

#include "battle/patch/battle.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/trainer_team.h"
#include "core/data_folder.h"
#include "core/problems.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/move_data.h"
#include "pokemon/patch/custom_shop.h"
#include "pokemon/patch/item_customizer.h"
#include "pokemon/patch/shiny.h"
#include "pokemon/patch/species_editor.h"
#include "script/patch/native_script.h"
#include "system/native/file.h"
#include "system/native/sound.h"
#include "ui/patch/starter_choice.h"
#include "ui/log.h"

namespace core {
namespace {

using maker::Condition;
using maker::Instruction;
using maker::Op;

void RunStory(script::Context& s);
void ApplyItemChange(pokemon::ItemData* item);
void ApplyMoveChange(MoveId move, pokemon::MoveData* data);

static_assert(maker::kAllShops == pokemon::CustomShop::kAnyShop,
              "the same id for all the shops");
static_assert(maker::kMaxShopItems == pokemon::ShopData::kMaxItems,
              "the same number of items in a shop");

// The problems of the last read of the files, for the Sango Maker page.
c8 problems[DataFiles::kMaxProblems][DataFiles::kMaxProblemLength];
u32 problem_count = 0;
// true while Reload() reads the files: the problems go to the log, not to
// the screen of the problems of the start.
bool is_reloading = false;

void AddProblem(const c8* format, ...) {
  if (problem_count < DataFiles::kMaxProblems) {
    va_list args;
    va_start(args, format);
    std::vsnprintf(problems[problem_count], DataFiles::kMaxProblemLength,
                   format, args);
    va_end(args);
  }
  problem_count++;
}

// Gives the parts of the files to the features of the game.
class GameSink : public maker::Sink {
public:
  void OnProblem(const c8* file, u32 line, const c8* what,
                 const c8* word) override {
    AddProblem("%s %u: %s %s", file, line, what, word);
    if (is_reloading) {
      ui::Log::Print(u"%s %u: %s %s", file, line, what, word);
    } else {
      Problems::Add(u"%s %u: %s %s", file, line, what, word);
    }
  }

  void OnEncounterTable(MapId map, EncounterMethod method,
                        const overworld::EncounterSlot* slots,
                        u32 count) override {
    overworld::WildEncounter::SetTable(map, method, slots, count);
  }

  void OnTrainer(TrainerId trainer) override {
    team_ = &teams_[team_count_++];
    *team_ = battle::TrainerTeam(Format::kSingle);
    trainer_ = trainer;
  }
  void OnTrainerFormat(Format format) override { team_->format = format; }
  void OnTrainerName(const c16* name) override { team_->WithName(name); }
  void OnTrainerMoney(u8 money) override { team_->WithMoney(money); }
  void OnTrainerAi(AiFlags flags) override { team_->WithAi(flags); }
  void OnTrainerItems(const ItemId* items, u32 count) override {
    ItemId all[4] = {ItemId::kNone, ItemId::kNone, ItemId::kNone,
                     ItemId::kNone};
    for (u32 i = 0; i < count && i < 4; i++) all[i] = items[i];
    team_->WithItems(all[0], all[1], all[2], all[3]);
  }
  void OnTrainerPokemon(const maker::Opponent& in) override {
    battle::TrainerOpponent opponent(in.species);
    if (in.has_level) opponent.WithLevel(in.level);
    if (in.has_item) opponent.WithItem(in.item);
    if (in.has_ability) opponent.WithAbility(in.ability);
    if (in.has_nature) opponent.WithNature(in.nature);
    if (in.has_form) opponent.WithForm(in.form);
    if (in.has_ivs) opponent.WithIvs(in.ivs);
    if (in.is_shiny) opponent.WithShiny();
    if (in.has_evs) {
      opponent.WithEvs(in.evs[0], in.evs[1], in.evs[2], in.evs[3], in.evs[4],
                       in.evs[5]);
    }
    if (in.has_moves) {
      opponent.WithMoves(in.moves[0], in.moves[1], in.moves[2], in.moves[3]);
    }
    team_->Add(opponent);
  }
  void OnTrainerEnd() override { battle::TrainerTeams::Add(trainer_, team_); }

  void OnSpeciesStats(SpeciesId species, FormId form,
                      const u8* stats) override {
    pokemon::Species(species, form)
        .WithStats(stats[0], stats[1], stats[2], stats[3], stats[4], stats[5]);
  }
  void OnSpeciesTypes(SpeciesId species, FormId form, TypeId first,
                      TypeId second) override {
    pokemon::Species(species, form).WithTypes(first, second);
  }
  void OnSpeciesAbilities(SpeciesId species, FormId form, AbilityId first,
                          AbilityId second, AbilityId hidden) override {
    pokemon::Species(species, form).WithAbilities(first, second, hidden);
  }
  void OnSpeciesCatchRate(SpeciesId species, FormId form, u8 rate) override {
    pokemon::Species(species, form).WithCatchRate(rate);
  }
  void OnSpeciesMachine(SpeciesId species, FormId form, u8 number, bool is_hm,
                        bool can_learn) override {
    if (is_hm) {
      pokemon::Species(species, form).WithHm(number, can_learn);
    } else {
      pokemon::Species(species, form).WithTm(number, can_learn);
    }
  }
  void OnSpeciesLearnset(SpeciesId species, FormId form,
                         const pokemon::LevelMove* moves, u32 count) override {
    pokemon::Species(species, form).WithLearnset(moves, count);
  }
  void OnSpeciesEvolutions(SpeciesId species,
                           const pokemon::EvolutionStep* steps,
                           u32 count) override {
    pokemon::Species(species).WithEvolutions(steps, count);
  }

  void OnStory(u16 id) override {
    const ScriptId script = static_cast<ScriptId>(id);
    // After Reload(), the script of the first read stays registered.
    if (is_reloading && script::NativeScript::IsRegistered(script)) return;
    script::NativeScript::Register(script, RunStory);
  }

  bool OnCharacter(const maker::Character& in) override {
    overworld::MapCharacterRequest request;
    request.map_id = in.map;
    request.model_id = in.model;
    request.tile_x = in.tile_x;
    request.tile_z = in.tile_z;
    request.height = static_cast<f32>(in.height);
    request.facing = static_cast<overworld::Facing>(in.facing);
    request.script_id = in.script;
    request.trainer_id = in.trainer;
    request.sight = in.sight;
    request.owner = this;
    return overworld::MapCharacter::Add(request);
  }

  void OnRule(maker::Rule rule, bool is_on) override {
    auto& battle = battle::Battle::GetInstance();
    switch (rule) {
      case maker::Rule::kItemsInBattle: battle.can_use_item = is_on; break;
      case maker::Rule::kOneMegaEvolution:
        battle.mega_restriction = is_on;
        break;
      case maker::Rule::kSameCatchRate:
        battle.same_ratio_for_all_pokeball = is_on;
        break;
      case maker::Rule::kInverseStats: battle.inverse_stats = is_on; break;
      case maker::Rule::kMetronomeOnly: battle.metronome_only = is_on; break;
      case maker::Rule::kTypeHelper: battle.show_type_helper = is_on; break;
      case maker::Rule::kRandomGifts:
        overworld::GiftPokemon::GetInstance().randomize_species = is_on;
        break;
      case maker::Rule::kNoEvLimit:
        pokemon::ItemCustomizer::GetInstance().remove_limit = is_on;
        break;
    }
  }

  void OnMultiplier(maker::Multiplier multiplier, u32 hundredths) override {
    auto& battle = battle::Battle::GetInstance();
    const f32 value = static_cast<f32>(hundredths) / 100.0f;
    switch (multiplier) {
      case maker::Multiplier::kExperience:
        battle.exp_multiplier = value;
        break;
      case maker::Multiplier::kCatchRate:
        battle.catch_multiplier = value;
        break;
      case maker::Multiplier::kMoney: battle.money_multiplier = value; break;
    }
  }

  void OnShinyRate(pokemon::ShinyRate rate) override {
    pokemon::Shiny::GetInstance().rate = rate;
  }

  void OnShopItems(u32 shop_id, const pokemon::ShopItem* items,
                   u32 count) override {
    pokemon::CustomShop::SetItems(items, count, shop_id);
  }

  void OnShopPokemons(u32 shop_id, const pokemon::ShopPokemon* pokemons,
                      u32 count) override {
    pokemon::CustomShop::SetPokemons(pokemons, count, shop_id);
    pokemon::CustomShop::EnablePokemonShop(true);
  }

  void OnItem(const maker::ItemChange& change) override {
    if (!item_changes_.Add(change)) {
      OnProblem("items.txt", 0, "too many items:", "256 at most");
      return;
    }
    pokemon::ItemCustomizer::GetInstance().on_item_data.Subscribe(
        ApplyItemChange);
  }

  void OnMove(const maker::MoveChange& change) override {
    if (!move_changes_.Add(change)) {
      OnProblem("moves.txt", 0, "too many moves:", "256 at most");
      return;
    }
    battle::GameExtension::GetInstance().on_move_data.Subscribe(
        ApplyMoveChange);
  }

  /// Returns the change of a move of moves.txt, or null.
  const maker::MoveChange* FindMoveChange(MoveId move) const {
    return move_changes_.Find([move](const maker::MoveChange& change) {
      return change.move == move;
    });
  }

  /// Returns the change of an item of items.txt, or null.
  const maker::ItemChange* FindItemChange(ItemId item) const {
    return item_changes_.Find([item](const maker::ItemChange& change) {
      return change.item == item;
    });
  }

  void OnStarters(const SpeciesId* species) override {
    auto& starters = ui::StarterChoice::GetInstance();
    for (u32 i = 0; i < ui::StarterChoice::kCount; i++) {
      starters.candidates[i] = species[i];
      starters.is_egg[i] = false;
    }
  }

  /// Returns true when the team comes from trainers.txt.
  bool IsOwnTeam(const battle::TrainerTeam* team) const {
    return team >= teams_ && team < teams_ + team_count_;
  }
  /// Forgets the teams. Remove them from TrainerTeams before.
  void ClearTeams() { team_count_ = 0; }
  /// Forgets the changes of items.txt and of moves.txt.
  void ClearChanges() {
    item_changes_.Clear();
    move_changes_.Clear();
  }

private:
  // The teams of trainers.txt: TrainerTeams keeps their addresses.
  battle::TrainerTeam teams_[256];
  u32 team_count_ = 0;
  battle::TrainerTeam* team_ = nullptr;
  TrainerId trainer_ = TrainerId::kNone;
  // The changes of items.txt.
  core::FixedList<maker::ItemChange, 256> item_changes_;
  // The changes of moves.txt.
  core::FixedList<maker::MoveChange, 256> move_changes_;
};

GameSink game_sink;

// Changes the data of an item that the game reads (items.txt).
void ApplyItemChange(pokemon::ItemData* item) {
  const maker::ItemChange* change = game_sink.FindItemChange(item->id);
  if (change == nullptr) return;
  if (change->has_price) item->price = change->price;
  if (change->has_fling_power) item->fling_power = change->fling_power;
  if (change->has_hp_restore) item->hp_restore_value = change->hp_restore;
  if (change->has_evs) {
    item->hp_ev_value = change->evs[0];
    item->attack_ev_value = change->evs[1];
    item->defense_ev_value = change->evs[2];
    item->sp_atk_ev_value = change->evs[3];
    item->sp_def_ev_value = change->evs[4];
    item->speed_ev_value = change->evs[5];
  }
}

// The text of a file of the SD card.
c8 file_buffer[maker::kMaxFileSize];

typedef void (*ReadFunction)(const c8* text, u32 size, maker::Sink& sink);

// Reads a file of the SD card and gives its parts to the game. Returns the
// size of the file (0: the file does not exist).
u32 LoadFile(const c16* path, const c8* name, ReadFunction read) {
  const u32 size = sys::File::ReadAll(path, file_buffer, sizeof(file_buffer));
  if (size == 0) return 0;
  if (size >= sizeof(file_buffer)) {
    game_sink.OnProblem(name, 0, "the file is too big", "");
  }
  read(file_buffer, size, game_sink);
  return size;
}

// The files of Load(), in their order.
struct DataFile {
  const c16* path;
  const c8* name;
  ReadFunction read;
};

const DataFile kFiles[] = {
    {SANGO_DATA_PATH(u"encounters.txt"), "encounters.txt",
     maker::ReadEncounters},
    {SANGO_DATA_PATH(u"trainers.txt"), "trainers.txt", maker::ReadTrainers},
    {SANGO_DATA_PATH(u"species.txt"), "species.txt", maker::ReadSpecies},
    {SANGO_DATA_PATH(u"story.txt"), "story.txt", maker::ReadStory},
    {SANGO_DATA_PATH(u"characters.txt"), "characters.txt",
     maker::ReadCharacters},
    {SANGO_DATA_PATH(u"rules.txt"), "rules.txt", maker::ReadRules},
    {SANGO_DATA_PATH(u"shops.txt"), "shops.txt", maker::ReadShops},
    {SANGO_DATA_PATH(u"items.txt"), "items.txt", maker::ReadItems},
    {SANGO_DATA_PATH(u"moves.txt"), "moves.txt", maker::ReadMoves},
};
constexpr u32 kFileCount = SIZE(kFiles);

// Update() reads one file each kPollFrames frames, when `is_live` is on.
constexpr u32 kPollFrames = 30;

// The hash of each file at the last read: a new hash means a change.
u32 file_hashes[kFileCount];
u32 frame = 0;
u32 reload_count = 0;
bool is_loaded = false;
bool is_reload_pending = false;

u32 HashBytes(const c8* data, u32 size) {
  u32 hash = 2166136261u ^ size;
  for (u32 i = 0; i < size; i++) {
    hash ^= static_cast<u8>(data[i]);
    hash *= 16777619u;
  }
  return hash;
}

// Removes the data of the last read from the game, before a new read. The
// changes of the stats, the types... of species.txt and of rules.txt stay.
void RemoveFileData() {
  overworld::WildEncounter::RemoveTables(
      [](const overworld::EncounterTable& table) {
        return maker::IsReaderMemory(table.slots);
      });
  battle::TrainerTeams::Remove([](const battle::TrainerTeamEntry& entry) {
    return game_sink.IsOwnTeam(entry.team);
  });
  game_sink.ClearTeams();
  pokemon::SpeciesChanges::RemoveLists(maker::IsReaderMemory);
  overworld::MapCharacter::RemoveByOwner(&game_sink);
  pokemon::CustomShop::RemoveItems([](const pokemon::ShopItem* items) {
    return maker::IsReaderMemory(items);
  });
  if (maker::IsReaderMemory(pokemon::CustomShop::GetPokemons())) {
    pokemon::CustomShop::SetPokemons(pokemon::kShopPokemons,
                                     SIZE(pokemon::kShopPokemons));
    pokemon::CustomShop::EnablePokemonShop(false);
  }
  game_sink.ClearChanges();
  maker::Clear();
}

// Changes the data of a move that the game loads (moves.txt).
void ApplyMoveChange(MoveId move, pokemon::MoveData* data) {
  const maker::MoveChange* change = game_sink.FindMoveChange(move);
  if (change == nullptr || data == nullptr) return;
  if (change->has_type) data->type = change->type;
  if (change->has_category) data->damage_category = change->category;
  if (change->has_power) data->power = change->power;
  if (change->has_accuracy) data->accuracy = change->accuracy;
  if (change->has_pp) data->base_pp = change->pp;
  if (change->has_priority) data->priority = change->priority;
}

// The values of the script that runs: the answers, the choice and the last
// battle.
struct StoryState {
  bool answer;
  s32 choice;
  script::BattleResult battle;
};

bool IsTrue(script::Context& s, const Instruction& in, const StoryState& st) {
  const u16 variable = in.condition >= Condition::kVarEqual &&
                               in.condition <= Condition::kVarLess
                           ? s.GetHackVariable(in.a)
                           : 0;
  switch (in.condition) {
    case Condition::kYes: return st.answer;
    case Condition::kNo: return !st.answer;
    case Condition::kChoice: return st.choice == static_cast<s32>(in.a);
    case Condition::kFlag: return s.GetHackFlag(in.a);
    case Condition::kNotFlag: return !s.GetHackFlag(in.a);
    case Condition::kVarEqual: return variable == in.b;
    case Condition::kVarNotEqual: return variable != in.b;
    case Condition::kVarAtLeast: return variable >= in.b;
    case Condition::kVarAtMost: return variable <= in.b;
    case Condition::kVarMore: return variable > in.b;
    case Condition::kVarLess: return variable < in.b;
    case Condition::kHasItem:
      return s.HasItem(static_cast<ItemId>(in.a), in.b);
    case Condition::kMoney:
      return s.GetMoney() >= (static_cast<u32>(in.b) << 16 | in.a);
    case Condition::kWon:
      return st.battle == script::BattleResult::kWon ||
             st.battle == script::BattleResult::kCaught;
    case Condition::kLost: return st.battle == script::BattleResult::kLost;
    default: return false;
  }
}

void RunStory(script::Context& s) {
  const u16 id = static_cast<u16>(script::NativeScript::GetCurrentId());
  const maker::Story* story = maker::FindStory(id);
  if (story == nullptr) return;

  StoryState state = {false, -1, script::BattleResult::kNotStarted};
  bool is_talking = false;
  u32 pc = story->first;
  const u32 end = story->first + story->count;
  while (pc < end) {
    const maker::Instruction& in = maker::GetInstruction(pc++);
    if (DataFiles::GetInstance().trace_story) {
      ui::Log::Print(u"story %u line %u: %s", id, in.line,
                     maker::GetCommandName(in.op));
    }
    switch (in.op) {
      case Op::kSay: s.Talk(in.text); break;
      case Op::kSign: s.Talk(in.text, script::WindowType::kSign); break;
      case Op::kAsk:
        s.Talk(in.text);
        state.answer = s.AskYesNo();
        break;
      case Op::kChoose: {
        const c16* choices[8];
        const c16* text = in.text;
        for (u32 k = 0; k < in.a; k++) {
          choices[k] = text;
          while (*text != 0) text++;
          text++;
        }
        const s32 choice = s.Choose(choices, in.a);
        // "if choice 1" is the first choice. 0: the player pressed B.
        state.choice = choice == script::kChoiceCancel ? 0 : choice + 1;
        break;
      }
      case Op::kIf:
        if (!IsTrue(s, in, state)) pc = in.jump;
        break;
      case Op::kElse: pc = in.jump; break;
      case Op::kEnd: break;
      case Op::kStop: pc = end; break;
      case Op::kFacePlayer:
        if (!is_talking) s.TalkStart();
        is_talking = true;
        break;
      case Op::kGiveItem: {
        const ItemId item = static_cast<ItemId>(in.a);
        if (s.CanGiveItem(item, in.b)) {
          s.GiveItem(item, in.b);
          s.PlayJingle(script::kJingleItem);
        }
        break;
      }
      case Op::kTakeItem: s.TakeItem(static_cast<ItemId>(in.a), in.b); break;
      case Op::kGivePokemon:
        if (s.GivePokemon(static_cast<SpeciesId>(in.a),
                          static_cast<u8>(in.b))) {
          s.PlayJingle(script::kJinglePokemon);
        }
        break;
      case Op::kGiveMoney: s.AddMoney(static_cast<u32>(in.b) << 16 | in.a); break;
      case Op::kTakeMoney: s.SubMoney(static_cast<u32>(in.b) << 16 | in.a); break;
      case Op::kSetFlag: s.SetHackFlag(in.a); break;
      case Op::kClearFlag: s.ResetHackFlag(in.a); break;
      case Op::kSetVariable: s.SetHackVariable(in.a, in.b); break;
      case Op::kAddVariable:
        savedata::HackSave::AddToVariable(in.a, in.b);
        break;
      case Op::kBattleTrainer:
        state.battle = s.TrainerBattle(static_cast<TrainerId>(in.a));
        break;
      case Op::kBattleWild:
        state.battle = s.WildBattle(static_cast<SpeciesId>(in.a),
                                    static_cast<u8>(in.b));
        break;
      case Op::kWarp:
        s.Warp(static_cast<MapId>(in.a), in.b, in.c, in.d,
               static_cast<overworld::Facing>(in.extra));
        break;
      case Op::kHeal: s.HealParty(); break;
      case Op::kFadeOut:
        s.FadeOut(in.extra != 0 ? script::FadeColor::kWhite
                                : script::FadeColor::kBlack);
        break;
      case Op::kFadeIn: s.FadeIn(); break;
      case Op::kMusic:
        s.StartEventMusic(static_cast<BackgroundMusicId>(in.a));
        break;
      case Op::kMusicEnd: s.EndEventMusic(); break;
      case Op::kWait: s.Wait(in.a); break;
      case Op::kSound:
        s.PlaySound(sys::Sound::kBankSoundEffect + in.a);
        break;
      case Op::kCallScript: s.CallScript(static_cast<ScriptId>(in.a)); break;
    }
  }
  if (is_talking) s.TalkEnd();
}

} // namespace

bool DataFiles::LoadEncounters(const c16* path) {
  return LoadFile(path, "encounters.txt", maker::ReadEncounters) != 0;
}

bool DataFiles::LoadTrainers(const c16* path) {
  return LoadFile(path, "trainers.txt", maker::ReadTrainers) != 0;
}

bool DataFiles::LoadSpecies(const c16* path) {
  return LoadFile(path, "species.txt", maker::ReadSpecies) != 0;
}

bool DataFiles::LoadStory(const c16* path) {
  return LoadFile(path, "story.txt", maker::ReadStory) != 0;
}

bool DataFiles::LoadCharacters(const c16* path) {
  return LoadFile(path, "characters.txt", maker::ReadCharacters) != 0;
}

void DataFiles::Load() {
  problem_count = 0;
  for (u32 i = 0; i < kFileCount; i++) {
    const DataFile& file = kFiles[i];
    const u32 size = LoadFile(file.path, file.name, file.read);
    file_hashes[i] = HashBytes(file_buffer, size);
  }
  is_loaded = true;
}

bool DataFiles::Reload() {
  // A script of story.txt reads the commands of the reader: wait for its end.
  if (script::NativeScript::GetCurrentId() != ScriptId::kNone) {
    is_reload_pending = true;
    return false;
  }
  is_reload_pending = false;
  RemoveFileData();
  is_reloading = true;
  Load();
  is_reloading = false;
  reload_count++;
  // The models of the characters of a region load with the map: a new
  // model needs a new load.
  overworld::MapCharacter::ReloadMapWhenPlayerMoves();
  ui::Log::Print(u"Sango Maker: files read again, %u problems",
                 problem_count);
  return true;
}

void DataFiles::Update() {
  if (is_reload_pending) {
    Reload();
    return;
  }
  if (!is_loaded || !GetInstance().is_live) return;
  if (++frame % kPollFrames != 0) return;
  // One file at each poll: a read of the SD card is slow.
  const u32 index = (frame / kPollFrames) % kFileCount;
  const u32 size = sys::File::ReadAll(kFiles[index].path, file_buffer,
                                      sizeof(file_buffer));
  if (HashBytes(file_buffer, size) != file_hashes[index]) Reload();
}

u32 DataFiles::GetProblemCount() { return problem_count; }

const c8* DataFiles::GetProblem(u32 index) {
  return problems[index < kMaxProblems ? index : 0];
}

u32 DataFiles::GetReloadCount() { return reload_count; }

bool DataFiles::IsLoaded() { return is_loaded; }

} // namespace core
