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
 * @file studio_trainers.cc
 * @brief What Sango Studio adds to the teams of the trainers.
 *
 * The declarations are in battle/patch/studio_trainers.h.
 */

#include "battle/patch/studio_trainers.h"

#include <cstring>

#include "battle/constant/field_effect_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/weather.h"
#include "battle/native/config.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/patch/studio_rules.h"
#include "pokemon/edit_pokemon.h"
#include "savedata/native/pokemon_team.h"
#include "system/native/file.h"

namespace battle {

namespace {

constexpr u32 kMaxFile = 12 + 56 * 6;
constexpr u32 kHeaderSize = 12;
constexpr u32 kPokemonSize = 56;
constexpr u32 kNicknameAt = 30;
constexpr u32 kNicknameLetters = 12;
constexpr u8 kUnchanged = 255;
constexpr u32 kNatureCount = 25;

/// The effects of the start of the battle that wait for the first Pokémon.
struct StartEffects {
  bool pending;
  bool listener_made;
  u8 terrain;
  u16 field;
};
StartEffects start_effects = {false, false, 0, 0};

u16 Read16(const u8* at) { return static_cast<u16>(at[0] | (at[1] << 8)); }

/// Adds an effect on the field, without a message.
void AddFieldEffect(Controller* controller, UID owner, FieldEffectKind effect,
                    TerrainKind terrain) {
  auto* mutation = static_cast<AddFieldEffectMutation*>(
      controller->Create(MutationKind::kAddFieldEffect, owner));
  mutation->effect = effect;
  mutation->terrain = terrain;
  mutation->duration.raw = 1;
  mutation->message.id = MutationMessageId::kNone;
  controller->Apply(mutation);
}

/// The first Pokémon that enters starts the effects: one time in the battle.
void StartReaction(Listener* self, Controller* controller, UID owner,
                   s32* local_state) {
  (void)self;
  (void)local_state;
  if (!start_effects.pending) return;
  start_effects.pending = false;
  if (start_effects.terrain != 0 &&
      start_effects.terrain < static_cast<u8>(TerrainKind::kCount)) {
    AddFieldEffect(controller, owner, FieldEffectKind::kTerrain,
                   static_cast<TerrainKind>(start_effects.terrain));
  }
  // The bits of the file: Trick Room, Gravity, Magic Room, Wonder Room.
  static const FieldEffectKind kEffects[] = {
      FieldEffectKind::kTrickRoom, FieldEffectKind::kGravity,
      FieldEffectKind::kMagicRoom, FieldEffectKind::kWonderRoom};
  for (u32 bit = 0; bit < 4; bit++) {
    if ((start_effects.field & (1u << bit)) != 0) {
      AddFieldEffect(controller, owner, kEffects[bit], TerrainKind::kNone);
    }
  }
}

const ReactionTable kStartTable[] = {
    {MomentKind::kPokemonEntered, StartReaction},
};

} // namespace

const ReactionTable* StudioTrainers::StartReactions(u32* count) {
  *count = 1;
  return kStartTable;
}

bool StudioTrainers::WantsStartListener() {
  return start_effects.pending && !start_effects.listener_made;
}

void StudioTrainers::StartListenerMade() {
  start_effects.listener_made = true;
}

void StudioTrainers::Apply(Config& config, TrainerId trainer) {
  start_effects = {false, false, 0, 0};
  c16 path[96];
  studio::MakePath(u"sdmc:/sango-studio/trainer/extra/",
                   static_cast<u32>(trainer), u".bin", path);
  static u8 file[kMaxFile];
  const u32 size = sys::File::ReadAll(path, file, sizeof(file));
  if (size < kHeaderSize || std::memcmp(file, "STRX", 4) != 0 || file[4] != 1) {
    return;
  }
  u32 count = file[5];
  if (count > 6 || size < kHeaderSize + kPokemonSize * count) return;

  if (file[6] != kUnchanged && file[6] < static_cast<u8>(Weather::kInvalid)) {
    config.weather = static_cast<Weather>(file[6]);
  }
  start_effects.terrain = file[7];
  start_effects.field = Read16(file + 8);
  start_effects.pending = start_effects.terrain != 0 || start_effects.field != 0;

  savedata::PokemonTeam* opponents = config.pokemon_teams[1];
  if (opponents == nullptr) return;
  if (count > opponents->count) count = opponents->count;
  pokemon::EditTeam edit(opponents->pokemons, count);
  for (u32 i = 0; i < count; i++) {
    const u8* record = file + kHeaderSize + kPokemonSize * i;
    pokemon::CoreData& pkm = edit[i];
    if (record[0] != kUnchanged && record[0] < kNatureCount) {
      pkm.nature = static_cast<Nature>(record[0]);
    }
    if (record[1] != 0) pkm.SetShiny(true);
    const u16 ability = Read16(record + 2);
    if (ability != 0) pkm.ability = static_cast<AbilityId>(ability);
    const u8* ev = record + 4;
    if (ev[0] | ev[1] | ev[2] | ev[3] | ev[4] | ev[5]) {
      pkm.SetStats(ev[0], ev[1], ev[2], ev[3], ev[4], ev[5]);
    }
    // The IVs: HP, Attack, Defense, Sp. Atk, Sp. Def, Speed.
    const u8* iv = record + 10;
    if (iv[0] != kUnchanged) pkm.iv_hp = iv[0] & 31;
    if (iv[1] != kUnchanged) pkm.iv_attack = iv[1] & 31;
    if (iv[2] != kUnchanged) pkm.iv_defense = iv[2] & 31;
    if (iv[3] != kUnchanged) pkm.iv_special_attack = iv[3] & 31;
    if (iv[4] != kUnchanged) pkm.iv_special_defense = iv[4] & 31;
    if (iv[5] != kUnchanged) pkm.iv_speed = iv[5] & 31;
    if (Read16(record + kNicknameAt) != 0) {
      c16 nickname[kNicknameLetters + 1];
      for (u32 letter = 0; letter < kNicknameLetters; letter++) {
        nickname[letter] = static_cast<c16>(Read16(record + kNicknameAt + 2 * letter));
      }
      nickname[kNicknameLetters] = 0;
      pkm.SetNickname(nickname);
    }
  }
}

} // namespace battle
