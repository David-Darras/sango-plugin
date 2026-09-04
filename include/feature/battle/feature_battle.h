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

#pragma once

#include "common.h"
#include "feature/battle/feature_game_extension.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/core/hook_manager.h"
#include "game/battle/manager.h"
#include "kaizo/kaizo.h"
#include "game/constant/move.h"
#include "game/renderer/app_layout_manager.h"

namespace feature {
struct Battle {
  MAKE_SINGLETON(Battle)
public:
  bool is_long_mega_evolve_animation = false;
  bool is_long_encounter_animation = false;
  bool show_enemy_pov = false;
  bool show_trainer_animation = false;
  bool show_pokeball_animation = false;
  bool show_fade_in = false;
  bool show_shiny_animation = false;

  bool no_shader = false;
  bool can_use_item = true;
  bool same_ratio_for_all_pokeball = false;
  bool fix_pokemon_size = true;
  bool sync_overworld_music = false;
  bool sync_team_hp = false;
  bool inverse_stats = false;
  bool metronome_only = false;

  bool mega_restriction = true;

  struct LevelUpData {
    u32 exp;

    u16 ev_hp;
    u16 ev_attack;

    u16 ev_defense;
    u16 ev_speed;

    u16 ev_special_attack;
    u16 ev_special_defense;

    bool _0;
    bool _1;
    bool use_exp_share;
    bool _2;
  };

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateExp, ADDRESS_BATTLE_UPDATE_EXP,
                            (uptr)UpdateExpHook, false);
    HookManager::Initialize(HookID::kStartMegaEvolveAnimation,
                            ADDRESS_BATTLE_START_MEGA_EVOLVE_ANIMATION,
                            (uptr)StartMegaEvolveAnimation, false);
    HookManager::Initialize(HookID::kStartBattleAnimation,
                            ADDRESS_BATTLE_START_BATTLE_ANIMATION,
                            (uptr)StartBattleAnimation, false);
    HookManager::Initialize(HookID::kStartBattleBackgroundMusic,
                            ADDRESS_BATTLE_START_BACKGROUND_MUSIC,
                            (uptr)StartBattleBackgroundMusicHook);
    HookManager::Initialize(HookID::kPlayBattleAnimation,
                            ADDRESS_BATTLE_PLAY_ANIMATION,
                            (uptr)PlayAnimationHook, false);
    HookManager::Initialize(HookID::kUpdateBattleView,
                            ADDRESS_UPDATE_BATTLE_VIEW,
                            (uptr)UpdateBattleViewHook, false);
    HookManager::Initialize(HookID::kBattleCheckPokemonCaptured,
                            ADDRESS_BATTLE_CHECK_POKEMON_CAPTURED,
                            (uptr)CheckPokemonCaptured,
                            false);
    HookManager::Initialize(HookID::kBattleUpdateGauge,
                            ADDRESS_BATTLE_UPDATE_GAUGE,
                            (uptr)UpdateGauge,
                            false);
    HookManager::Initialize(HookID::kBattleUpdateView,
                            ADDRESS_BATTLE_UPDATE_VIEW,
                            (uptr)UpdateView,
                            false);
  }

  STATIC_INLINE void PatchUpdate() {
    static u32 counter = 20;
    counter--;
    if (counter != 0) return;
    counter = 20;

    auto& feat = GetInstance();

    if (feat.inverse_stats) {
      auto& server_team = battle::Manager::GetTeam(true, 0);
      auto& client_team = battle::Manager::GetTeam(false, 0);
      for (u32 i = 0; i < server_team.count; i++) {
        server_team.pokemon[i]->InverseStats();
        client_team.pokemon[i]->InverseStats();
      }
    }

    if (feat.metronome_only) {
      for (u32 j = 0; j < 2; j++) {
        auto& server_team = battle::Manager::GetTeam(true, j);
        auto& client_team = battle::Manager::GetTeam(false, j);
        for (u32 i = 0; i < server_team.count; i++) {
          server_team.pokemon[i]->SetMetronome();
          client_team.pokemon[i]->SetMetronome();
        }
      }
    }

    if (feat.sync_team_hp) {
      bool kill_all = false;
      auto& server_team = battle::Manager::GetTeam(true, 0);
      auto& client_team = battle::Manager::GetTeam(false, 0);
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


  STATIC_INLINE void PatchLoad() {
    MEMORY_SCOPE(ADDRESS_MEMORY_REGION_GAME_CODE, 0xD8000);
    HookManager::ForceEnable(HookID::kUpdateBattleView);
    HookManager::ForceEnable(HookID::kUpdateExp);
    HookManager::ForceEnable(HookID::kStartMegaEvolveAnimation);
    HookManager::ForceEnable(HookID::kStartBattleAnimation);
    HookManager::ForceEnable(HookID::kPlayBattleAnimation);
    HookManager::ForceEnable(HookID::kBattleCheckPokemonCaptured);
    HookManager::ForceEnable(HookID::kBattleUpdateGauge);
    HookManager::ForceEnable(HookID::kBattleUpdateView);
    GameExtension::PatchBattleLoad();

    auto& feat = GetInstance();
    if (!feat.can_use_item) {
      // Only access to pokeball
      WRITE8(ADDRESS_BATTLE_MENU_ENTRY_HP_PP, 2); // HP/PP -> Ball
      // WRITE8(0x007CB0B4, 2) // Ball
      WRITE8(ADDRESS_BATTLE_MENU_ENTRY_STATUS, 2); // Status -> Ball
      WRITE8(ADDRESS_BATTLE_MENU_ENTRY_BATTLE, 2); // Battle -> Item
    }

    if (feat.same_ratio_for_all_pokeball) {
      // Disable master ball feature
      ARM_NOP(ADDRESS_BATTLE_MASTER_BALL_CHECK);
      ARM_NO_COND(ADDRESS_BATTLE_MASTER_BALL_BRANCH);

      // same ratio for all balls
      WRITE32(ADDRESS_BATTLE_BALL_CATCH_RATE, 0xE3A00A01); // mov r0, #0x1000
      ARM_RET(ADDRESS_BATTLE_BALL_CATCH_RATE_RETURN);
    }

    if (feat.no_shader) {
      ARM_RET(ADDRESS_RENDERER_APPLY_SHADER);
      ARM_RET(ADDRESS_RENDERER_APPLY_SHADER_2);
    }

    if (!feat.mega_restriction) {
      ARM_NOP(ADDRESS_BATTLE_MEGA_RESTRICTION_CHECK);
      ARM_NOP(ADDRESS_BATTLE_MEGA_RESTRICTION_CHECK_2);
    }
  }

  static bool CheckPokemonCaptured(u32 p0, u32 p1, u32 p2, u32 p3, u32 p4,
                                   u32 p5) {
#ifdef KAIZO
    // wild battle -> trainer battle (disable capture)
    u8 battle_type = READ8(READ32(READ32(p1 + 4) + 0x10));
    if (kaizo::CapturedEvent::Check()) {
      WRITE8(READ32(READ32(p1 + 4) + 0x10), 1);
    }
#endif
    bool result = HookManager::Call<bool>(HookID::kBattleCheckPokemonCaptured,
                                          p0, p1, p2, p3, p4, p5);
#ifdef KAIZO
    if (result) {
      kaizo::CapturedEvent::Set();
    } else {
      WRITE8(READ32(READ32(p1 + 4) + 0x10), battle_type);
    }
#endif

    return result;
  }

  static Color8 LerpColor(Color8 a, Color8 b, f32 t) {
    Color8 c;
    c.r = (u8)(a.r + (b.r - a.r) * t);
    c.g = (u8)(a.g + (b.g - a.g) * t);
    c.b = (u8)(a.b + (b.b - a.b) * t);
    c.a = 255;
    return c;
  }

  static Color8 GetHpGaugeColor(f32 ratio) {
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

  static void UpdateGauge(uptr gauge, u16 max_hp, u32 new_hp) {
    HookManager::Call<void>(HookID::kBattleUpdateGauge, gauge, max_hp, new_hp);
    uptr res = ((uptr(*)(uptr))ADDRESS_BATTLE_HP_GAUGE_GET_PANE)(READ32(gauge + 48));

    f32 ratio = (max_hp > 0) ? (f32)new_hp / (f32)max_hp : 0.0f;
    Color8 color = GetHpGaugeColor(ratio);

    WRITE32(res + 16, color.GetRaw());
  }

  static void UpdateBattleViewHook(uptr p0) {
    if (GetInstance().fix_pokemon_size) {
      static u32 counter = 20;
      if (counter >= 20) {
        PatchPokemonSize();
        counter = 0;
      }
      counter++;
    }
    HookManager::Call<void>(HookID::kUpdateBattleView, p0);
  }

  static void PatchPokemonSize() {
    for (u32 i = 0; i < 6; i++) {
      u32 pkmMdl = READ32(ADDRESS_BATTLE_POKEMON_MODEL_TABLE + 4 * i); // Base address of the 3D model
      if (pkmMdl == 0)
        continue;

      u16 pkmNum = READ16(pkmMdl + 0x170); // Pokédex number
      if (pkmNum >= 722)
        continue;

      u32 pkmData = READ32(ADDRESS_GLOBAL_DATA_POKEMON_TABLE) + 0x50 * pkmNum;
      // Personal data of the Pokémon
      f32 realSize = (f32)READ16(pkmData + 0x24);
      // Actual height according to the Pokédex
      f32 defaultSize = (f32)READ16(pkmData + 0x3C);
      // Default displayed size in battle
      f32 ratio = realSize / defaultSize;

      // Update Pokémon scale
      WRITEF(pkmMdl + 0x34, ratio);
      WRITEF(pkmMdl + 0x38, ratio);
      WRITEF(pkmMdl + 0x3C, ratio);

      // Mark the model to be updated
      WRITEF(pkmMdl + 0x3C, ratio);
      WRITEB(pkmMdl + 0x4C, true);
    }
  }

  static void PlayAnimationHook(uptr view_manager, u16 id) {
    // disable shiny effect
    if (id == 621 && !GetInstance().show_shiny_animation) return;
    return HookManager::Call<void>(HookID::kPlayBattleAnimation, view_manager,
                                   id);
  }

  static void
  StartBattleBackgroundMusicHook(uptr sound_manager, u32 id, u8 p2) {
    if (GetInstance().sync_overworld_music) {
      id = (1 << 16) + Overworld::GetInstance().background_music;
    }
    return HookManager::Call<void>(HookID::kStartBattleBackgroundMusic,
                                   sound_manager, id, p2);
  }

  static void StartBattleAnimation(void* p0, void* p1) {
    struct BattleAnimationData {
      u32 view;
      u32 _0;
      u32 state;
      u8 position[5];
      u32 trainer_model[2];

      bool skip_pokeball_animation;
      bool is_long_encounter_animation;
      bool use_trainer_pov;
      bool show_fade_in;
      bool show_shiny_animation;
      bool dont_show_trainer;
    }* data = (BattleAnimationData*)p1;
    auto& config = GetInstance();
    data->skip_pokeball_animation = !config.show_pokeball_animation;
    data->is_long_encounter_animation = config.is_long_encounter_animation;
    data->use_trainer_pov = !config.show_enemy_pov;
    data->show_fade_in = config.show_fade_in;
    data->skip_pokeball_animation = !config.show_pokeball_animation;
    data->show_shiny_animation = config.show_shiny_animation;
    data->dont_show_trainer = !config.show_trainer_animation;

    HookManager::Call<void>(HookID::kStartBattleAnimation, p0, p1);
  }

  static void StartMegaEvolveAnimation(void* view, u8 target,
                                       bool is_long_animation) {
    HookManager::Call<void>(HookID::kStartMegaEvolveAnimation, view, target,
                            GetInstance().is_long_mega_evolve_animation);
  }

  static bool UpdateExpHook(void* self, battle::Team* team,
                            LevelUpData* data) {
#ifdef KAIZO
    // kaizo::ApplyLevelCaps(team, data);
#endif
    return HookManager::Call<bool>(HookID::kUpdateExp, self, team, data);
  }

  static void UpdateView(uptr self) {
    u32* camera = *(u32**)(self + 408);
    camera[4] = 0; // don't use split view
    camera[5] = 0x7FFFFFFF; // disable camera animation
    return HookManager::Call<void>(HookID::kBattleUpdateView, self);
  }
};
} // namespace feature