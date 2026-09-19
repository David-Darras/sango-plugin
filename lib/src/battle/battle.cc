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

#include "battle/patch/battle.h"

#include "battle/native/manager.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/type_chart.h"
#include "core/hook_manager.h"
#include "overworld/patch/field.h"
#include "system/native/sound.h"
#include "pokemon/native/species_data.h"

namespace battle {

void Battle::Initialize() {
  core::HookManager::Initialize(HookId::kBattleLevelUp, address::kLevelUp,
                          (uptr)LevelUpHook, false);
  core::HookManager::Initialize(HookId::kBattleStartMegaEvolutionAnimation,
                          address::kStartMegaEvolutionAnimation,
                          (uptr)StartMegaEvolutionAnimationHook, false);
  core::HookManager::Initialize(HookId::kBattleStartEntryAnimation,
                          address::kStartEntryAnimation,
                          (uptr)StartEntryAnimationHook, false);
  core::HookManager::Initialize(HookId::kBattleStartBackgroundMusic,
                          address::kStartBackgroundMusic,
                          (uptr)StartBackgroundMusicHook);
  core::HookManager::Initialize(HookId::kBattlePlayAnimation,
                          address::kPlayAnimation,
                          (uptr)PlayAnimationHook, false);
  core::HookManager::Initialize(HookId::kBattleUpdateView,
                          address::kUpdateView,
                          (uptr)UpdateViewHook, false);
  core::HookManager::Initialize(HookId::kBattleCheckPokemonCaptured,
                          pokemon::address::kBattleCheckPokemonCaptured,
                          (uptr)CheckPokemonCapturedHook,
                          false);
  core::HookManager::Initialize(HookId::kBattleUpdateGauge,
                          address::kUpdateGauge,
                          (uptr)UpdateGaugeHook,
                          false);
}

void Battle::PatchUpdate() {
  const u32 kMax = 20;
  static u32 counter = 0;
  counter--;
  if (counter != 0) return;
  counter = kMax;

  auto& feat = GetInstance();

  if (feat.inverse_stats) {
    auto& server_team = Manager::GetTeam(true, 0);
    auto& client_team = Manager::GetTeam(false, 0);
    for (u32 i = 0; i < server_team.count; i++) {
      server_team.pokemon[i]->InverseStats();
      client_team.pokemon[i]->InverseStats();
    }
  }

  if (feat.metronome_only) {
    for (u32 j = 0; j < 2; j++) {
      auto& server_team = Manager::GetTeam(true, j);
      auto& client_team = Manager::GetTeam(false, j);
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->SetMetronome();
        client_team.pokemon[i]->SetMetronome();
      }
    }
  }

  if (!feat.mega_restriction) {
    Manager::GetInstance().ResetMegaEvolutions();
  }

  if (feat.sync_team_hp) {
    bool kill_all = false;
    auto& server_team = Manager::GetTeam(true, 0);
    auto& client_team = Manager::GetTeam(false, 0);
    for (u32 i = 0; i < server_team.count; i++) {
      auto& pkm = *server_team.pokemon[i];
      if (pkm.hp == 0) {
        kill_all = true;
        break;
      }
    }
    if (kill_all) {
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->hp = 0;
        client_team.pokemon[i]->hp = 0;
      }
    }
  }
}

void Battle::PatchLoad() {
  MEMORY_SCOPE(sys::address::kMemoryRegionGameCode, 0xD8000);
  core::HookManager::ForceEnable(HookId::kBattleUpdateView);
  core::HookManager::ForceEnable(HookId::kBattleLevelUp);
  core::HookManager::ForceEnable(HookId::kBattleStartMegaEvolutionAnimation);
  core::HookManager::ForceEnable(HookId::kBattleStartEntryAnimation);
  core::HookManager::ForceEnable(HookId::kBattlePlayAnimation);
  core::HookManager::ForceEnable(HookId::kBattleCheckPokemonCaptured);
  core::HookManager::ForceEnable(HookId::kBattleUpdateGauge);
  GameExtension::PatchBattleLoad();
  TypeChart::PatchLoad();

  auto& feat = GetInstance();
  if (!feat.can_use_item) {
    WRITE8(address::kMenuEntryHpPp, 2);
    WRITE8(address::kMenuEntryStatus, 2);
    WRITE8(address::kMenuEntryBattle, 2);
  }

  if (feat.same_ratio_for_all_pokeball) {
    ARM_NOP(address::kMasterBallCheck);
    ARM_NO_COND(address::kMasterBallBranch);

    WRITE32(address::kBallCatchRate, 0xE3A00A01); // mov r0, #0x1000
    ARM_RET(address::kBallCatchRate + 4);
  }

  if (feat.no_shader) {
    ARM_RET(renderer::address::kApplyShader);
    ARM_RET(renderer::address::kApplyShader2);
  }

  if (!feat.mega_restriction) {
    ARM_NOP(address::kMegaRestrictionCheck);
    ARM_NOP(address::kMegaRestrictionCheck2);

    if (feat.unlimited_mega_evolution) {
      WRITE32(address::kCanMegaEvolved, 0xE3A00001);
      ARM_RET(address::kCanMegaEvolved + 4);
    }
  }
}

bool Battle::CheckPokemonCapturedHook(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                      u32 p5) {
  auto& feat = GetInstance();
  u8* battle_type = (u8*)(READ32(READ32(p1 + 4) + 0x10));
  const u8 original_type = *battle_type;
  if (feat.is_capture_allowed != nullptr && !feat.is_capture_allowed()) {
    *battle_type = 1;
  }
  bool result = core::HookManager::Call<bool>(HookId::kBattleCheckPokemonCaptured,
                                        p0, p1, p2, p3, p4, p5);
  if (result) {
    if (feat.on_captured != nullptr) feat.on_captured();
  } else {
    *battle_type = original_type;
  }

  return result;
}

Color8 Battle::LerpColor(Color8 a, Color8 b, f32 t) {
  Color8 c;
  c.r = a.r + (u8)((f32)(b.r - a.r) * t);
  c.g = a.g + (u8)((f32)(b.g - a.g) * t);
  c.b = a.b + (u8)((f32)(b.b - a.b) * t);
  c.a = 255;
  return c;
}

Color8 Battle::GetHpGaugeColor(f32 ratio) {
  if (ratio < 0.0f) ratio = 0.0f;
  if (ratio > 1.0f) ratio = 1.0f;

  Color8 magenta = {255, 0, 180, 255};
  Color8 red = {255, 0, 60, 255};
  Color8 orange = {255, 100, 0, 255};
  Color8 yellow = {255, 230, 0, 255};
  Color8 green = {0, 255, 90, 255};
  Color8 cyan = {0, 230, 255, 255};

  if (ratio < 0.2f) {
    return LerpColor(magenta, red, ratio / 0.2f);
  } else if (ratio < 0.4f) {
    return LerpColor(red, orange, (ratio - 0.2f) / 0.2f);
  } else if (ratio < 0.6f) {
    return LerpColor(orange, yellow, (ratio - 0.4f) / 0.2f);
  } else if (ratio < 0.8f) {
    return LerpColor(yellow, green, (ratio - 0.6f) / 0.2f);
  } else {
    return LerpColor(green, cyan, (ratio - 0.8f) / 0.2f);
  }
}

void Battle::UpdateGaugeHook(uptr gauge, u16 max_hp, u32 new_hp) {
  core::HookManager::Call<void>(HookId::kBattleUpdateGauge, gauge, max_hp, new_hp);
  uptr res = ((uptr(*)(uptr))address::kGetHpGaugePane)(
      READ32(gauge + 48));

  f32 ratio = (max_hp > 0) ? (f32)new_hp / (f32)max_hp : 0.0f;
  Color8 color = GetHpGaugeColor(ratio);

  WRITE32(res + 16, color.GetRaw());
}

void Battle::UpdateViewHook(uptr self) {
  if (kCameraOffset != 0) {
    u32* camera = *(u32**)(self + kCameraOffset);
    camera[4] = 0; // don't use split view
    camera[5] = 0x7FFFFFFF; // disable camera animation
  }

  if (GetInstance().fix_pokemon_size) {
    static u32 counter = 20;
    if (counter >= 20) {
      PatchPokemonSize();
      counter = 0;
    }
    counter++;
  }
  core::HookManager::Call<void>(HookId::kBattleUpdateView, self);
}

void Battle::PatchPokemonSize() {
  for (u32 i = 0; i < 6; i++) {
    u32 pkmMdl = READ32(address::kPokemonModelTable + 4 * i);
    if (pkmMdl == 0)
      continue;

    u16 pkmNum = READ16(pkmMdl + 0x170);
    if (pkmNum >= 722)
      continue;

    const pokemon::SpeciesData& data = pokemon::SpeciesData::GetTable()[pkmNum];
    f32 realSize = (f32)data.height;
    f32 defaultSize = (f32)data.fake_height;
    f32 ratio = realSize / defaultSize;

    WRITEF(pkmMdl + 0x34, ratio);
    WRITEF(pkmMdl + 0x38, ratio);
    WRITEF(pkmMdl + 0x3C, ratio);

    WRITEB(pkmMdl + 0x4C, true);
  }
}

void Battle::PlayAnimationHook(uptr view_manager, u16 id) {
  if (id == kShinyAnimationId && !GetInstance().show_shiny_animation) return;
  return core::HookManager::Call<void>(HookId::kBattlePlayAnimation, view_manager, id);
}

void Battle::StartBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2) {
  if (GetInstance().sync_overworld_music) {
    id = sys::Sound::kBankBackgroundMusic +
         static_cast<u32>(overworld::Field::GetInstance().background_music);
  }
  return core::HookManager::Call<void>(HookId::kBattleStartBackgroundMusic,
                                 sound_manager, id, p2);
}

void Battle::StartEntryAnimationHook(void* p0, EntryAnimationData* data) {
  auto& config = GetInstance();
  data->skip_pokeball_animation = !config.show_pokeball_animation;
  data->is_long_encounter_animation = config.is_long_encounter_animation;
  data->use_trainer_pov = !config.show_enemy_pov;
  data->show_fade_in = config.show_fade_in;
  data->skip_pokeball_animation = !config.show_pokeball_animation;
  data->show_shiny_animation = config.show_shiny_animation;
  if (kHasTrainerVisibilityFlag) data->dont_show_trainer = !config.show_trainer_animation;

  core::HookManager::Call<void>(HookId::kBattleStartEntryAnimation, p0, data);
}

void Battle::StartMegaEvolutionAnimationHook(void* view, u8 target,
                                            bool is_long_animation) {
  core::HookManager::Call<void>(HookId::kBattleStartMegaEvolutionAnimation, view,
                          target,
                          GetInstance().is_long_mega_evolve_animation);
}

bool Battle::LevelUpHook(void* self, Team* team,
                        LevelUpData* data) {
  return core::HookManager::Call<bool>(HookId::kBattleLevelUp, self, team, data);
}

} // namespace battle
