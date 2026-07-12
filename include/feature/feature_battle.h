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
#include "hook_manager.h"
#include "game/battle/manager.h"
#include "game/savedata/event_table.h"
#include "game/savedata/misc.h"
#include "game/savedata/pokemon_utils.h"

#define ADDRESS_BATTLE_UPDATE_EXP (0x0075D73C)
#define ADDRESS_BATTLE_START_MEGA_EVOLVE_ANIMATION (0x0072EF18)
#define ADDRESS_BATTLE_START_BATTLE_ANIMATION (0x00775354)

namespace feature {
struct Battle {
  MAKE_SINGLETON(Battle)
public:
  bool is_long_mega_evolve_animation = true;
  bool is_long_encounter_animation = false;
  bool show_enemy_pov = false;
  bool show_trainer_animation = true;
  bool show_pokeball_animation = true;
  bool show_fade_in = false;
  bool show_shiny_animation = false;

  STATIC_INLINE void Initialize() {
    // HookManager::GetInstance().Add(HookID::kOnStartTurn, 0x00759B74,
    //                                (uptr)OnStartTurn);
    //
    //
    // HookManager::GetInstance().Add(HookID::kPlayBattleAnimation, 0x007510A8,
    //                                (uptr)PlayBattleAnimation);
    //
    // HookManager::GetInstance().Add(HookID::kOnLoadTrainerModel, 0x00458214,
    //                                (uptr)OnLoadTrainerModel);
    HookManager::Initialize(HookID::kUpdateExp, ADDRESS_BATTLE_UPDATE_EXP,
                            (uptr)UpdateExpHook, false);
    HookManager::Initialize(HookID::kStartMegaEvolveAnimation,
                            ADDRESS_BATTLE_START_MEGA_EVOLVE_ANIMATION,
                            (uptr)StartMegaEvolveAnimation, false);
    HookManager::Initialize(HookID::kStartBattleAnimation,
                            ADDRESS_BATTLE_START_BATTLE_ANIMATION,
                            (uptr)StartBattleAnimation, false);
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

  static bool UpdateExpHook(void* self, battle::Team* team,
                            LevelUpData* data) {
    static const u8 LEVEL_CAPS[] = {
        17, // 0 badge
        19, // 1 badge
        28, // 2 badges
        38, // 3 badges
        42, // 4 badges
        50, // 5 badges
        62, // 6 badges
        67, // 7 badges
        80 // 8 badges
    };

    u32 count = savedata::Misc::GetInstance().GetBadgesCount();
    u32 max_level = LEVEL_CAPS[count];

    for (u32 i = 0; i < team->count; i++) {
      data[i].ev_hp = 0;
      data[i].ev_attack = 0;
      data[i].ev_defense = 0;
      data[i].ev_speed = 0;
      data[i].ev_special_attack = 0;
      data[i].ev_special_defense = 0;

      // Game finished => Max Level = 100
      if (!savedata::EventTable::GetInstance().Check(2720)) {
        u8 new_level = PokemonUtils::GetLevelFromExperience(
            team->pokemon[i]->species,
            team->pokemon[i]->form,
            team->pokemon[i]->experience + data[i].exp);
        if (new_level >= max_level) {
          data[i].exp = 0;
        }
      }
    }

    return HookManager::Call<bool>(HookID::kUpdateExp, self, team, data);
  }

  // static void LoadTrainerModelHook(uptr trainer_model,
  //                                  void* trainer_model_manager) {
  //   *(u16*)trainer_model = ctx.trainer_model_id;
  //   *(u16*)(trainer_model + 2) = 0;
  //
  //   return HookManager::GetInstance()
  //          .Get(HookID::kOnLoadTrainerModel)
  //          ->CallOriginal<void>(trainer_model, trainer_model_manager);
  // }

  // static void PlayAnimationHook(void* graphics, u16 animation) {
  //   // shiny anim -> rainbow anim
  //   if (animation == 621) {
  //     animation = 635;
  //   }
  //   HookManager::GetInstance()
  //       .Get(HookID::kPlayBattleAnimation)
  //       ->CallOriginal<void>(graphics, animation);
  // }
  //
  // static u32 StartTurnHook(uptr server, u32 action) {
  //   // uptr server = READ(u32, 0x082061F8);
  //
  //   // Change Ability
  //   {
  //     struct {
  //       u32 flags;
  //       u16 ability;
  //       u8 id;
  //       u8 _0;
  //       u8 _1;
  //       u32 message[9];
  //     } cmd{};
  //     cmd.id = 0;
  //
  //     cmd.ability = 191;
  //     ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
  //
  //     cmd.ability = 189;
  //     ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
  //   }
  //
  //   // Change HP
  //   {
  //     struct {
  //       u32 flags;
  //       u8 count;
  //       u8 _0[2];
  //       u8 id[10];
  //       s32 hp[10];
  //     } cmd{};
  //
  //     cmd.id[0] = 0;
  //     cmd.count = 1;
  //
  //     cmd.hp[0] = -200;
  //     ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
  //
  //     cmd.hp[0] = 150;
  //     ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
  //   }
  //
  //   // Kill
  //   {
  //     struct {
  //       u32 flags;
  //       u8 id;
  //       u8 _0;
  //       u16 _1;
  //       u32 message[9];
  //     } cmd{};
  //
  //     cmd.id = 0;
  //     ((void (*)(uptr, void*))0x0710D3C)(server, &cmd);
  //   }
  //
  //   // Revive
  //   {
  //     struct {
  //       u32 flags;
  //       u8 id;
  //       u16 hp;
  //       u32 message[9];
  //     } cmd{};
  //
  //     cmd.id = 0;
  //     cmd.hp = 999;
  //
  //     ((void (*)(uptr, void*))0x00711F08)(server, &cmd);
  //   }
  //
  //   // Metamorph
  //   {
  //     struct {
  //       u32 flags : 8;
  //       u32 source : 5;
  //       u32  : 19;
  //       u8 target;
  //       u32 msg[9];
  //     } cmd{};
  //
  //     cmd.source = 0;
  //     cmd.target = 12;
  //
  //     ((void (*)(uptr, void*))0x00711CE4)(server, &cmd);
  //   }
  //
  //   // Attack
  //   {
  //     struct {
  //       u32 action : 4;
  //       u32 target : 4;
  //       u32 move : 16;
  //     } cmd{};
  //
  //     cmd.action = 1; // attack
  //     cmd.target = 1; // enemy
  //     cmd.move = 588;
  //
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //
  //     cmd.move = 604;
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //
  //     cmd.move = 617;
  //     ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
  //         server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  //   }
  //
  //   return HookManager::GetInstance()
  //          .Get(HookID::kOnStartTurn)
  //          ->CallOriginal<u32>(server, action);
  // }
};
} // namespace feature