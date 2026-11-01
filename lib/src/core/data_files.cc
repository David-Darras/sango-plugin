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

#include "battle/patch/trainer_team.h"
#include "core/data_folder.h"
#include "core/problems.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/patch/species_editor.h"
#include "script/patch/native_script.h"
#include "system/native/file.h"
#include "system/native/sound.h"

namespace core {
namespace {

using maker::Condition;
using maker::Instruction;
using maker::Op;

void RunStory(script::Context& s);

// Gives the parts of the files to the features of the game.
class GameSink : public maker::Sink {
public:
  void OnProblem(const c8* file, u32 line, const c8* what,
                 const c8* word) override {
    Problems::Add(u"%s %u: %s %s", file, line, what, word);
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
    script::NativeScript::Register(static_cast<ScriptId>(id), RunStory);
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
    return overworld::MapCharacter::Add(request);
  }

private:
  // The teams of trainers.txt: TrainerTeams keeps their addresses.
  battle::TrainerTeam teams_[256];
  u32 team_count_ = 0;
  battle::TrainerTeam* team_ = nullptr;
  TrainerId trainer_ = TrainerId::kNone;
};

GameSink game_sink;

// The text of a file of the SD card.
c8 file_buffer[maker::kMaxFileSize];

typedef void (*ReadFunction)(const c8* text, u32 size, maker::Sink& sink);

// Reads a file of the SD card and gives its parts to the game. Returns false
// when the file does not exist.
bool LoadFile(const c16* path, const c8* name, ReadFunction read) {
  const u32 size = sys::File::ReadAll(path, file_buffer, sizeof(file_buffer));
  if (size == 0) return false;
  if (size >= sizeof(file_buffer)) {
    Problems::Add(u"%s: the file is too big", name);
  }
  read(file_buffer, size, game_sink);
  return true;
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
  return LoadFile(path, "encounters.txt", maker::ReadEncounters);
}

bool DataFiles::LoadTrainers(const c16* path) {
  return LoadFile(path, "trainers.txt", maker::ReadTrainers);
}

bool DataFiles::LoadSpecies(const c16* path) {
  return LoadFile(path, "species.txt", maker::ReadSpecies);
}

bool DataFiles::LoadStory(const c16* path) {
  return LoadFile(path, "story.txt", maker::ReadStory);
}

bool DataFiles::LoadCharacters(const c16* path) {
  return LoadFile(path, "characters.txt", maker::ReadCharacters);
}

void DataFiles::Load() {
  LoadEncounters(SANGO_DATA_PATH(u"encounters.txt"));
  LoadTrainers(SANGO_DATA_PATH(u"trainers.txt"));
  LoadSpecies(SANGO_DATA_PATH(u"species.txt"));
  LoadStory(SANGO_DATA_PATH(u"story.txt"));
  LoadCharacters(SANGO_DATA_PATH(u"characters.txt"));
}

} // namespace core
