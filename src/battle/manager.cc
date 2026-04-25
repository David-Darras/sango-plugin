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

#include "battle/manager.h"

#include <math.h>

#include <cstring>

#include "hack/hook_manager.h"
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
namespace battle {

static struct {
  u8 team_idx;
  u8 pokemon_idx;
  Pokemon* pkm_server;
  Pokemon* pkm_client;
} ctx{};

void OnSave(void*) {
  for (u32 i = 0; i < 4; i++) {
    memcpy(&ctx.pkm_server->moves[i].core, &ctx.pkm_server->moves[i].view,
           sizeof(ctx.pkm_server->moves[i].view));
  }
  memcpy(ctx.pkm_client, ctx.pkm_server, sizeof(Pokemon));
}

void Pokemon::LoadMenu(menu::PluginMenu& menu, void* args) {
  Pokemon& pkm = *ctx.pkm_server;

  menu.Add("Save", OnSave)
      .Add("UID", pkm.uid)
      .AddSpecies("Species", pkm.species)

      .AddMove("Move 1 - ID", pkm.moves[0].view.id)
      .Add("Move 1 - PP", pkm.moves[0].view.pp)
      .Add("Move 1 - Max PP", pkm.moves[0].view.max_pp)

      .AddMove("Move 2 - ID", pkm.moves[1].view.id)
      .Add("Move 2 - PP", pkm.moves[1].view.pp)
      .Add("Move 2 - Max PP", pkm.moves[1].view.max_pp)

      .AddMove("Move 3 - ID", pkm.moves[2].view.id)
      .Add("Move 3 - PP", pkm.moves[2].view.pp)
      .Add("Move 3 - Max PP", pkm.moves[2].view.max_pp)

      .AddMove("Move 4 - ID", pkm.moves[3].view.id)
      .Add("Move 4 - PP", pkm.moves[3].view.pp)
      .Add("Move 4 - Max PP", pkm.moves[3].view.max_pp)

      .Add("Hp", pkm.hp)
      .Add("Max Hp", pkm.max_hp)

      .Add("Attack", pkm.attack)
      .Add("Defense", pkm.defense)
      .Add("Sp. Attack", pkm.special_attack)
      .Add("Sp. Defense", pkm.special_defense)
      .Add("Speed", pkm.speed)

      .Add("Atk Stage", pkm.stat_attack)
      .Add("Def Stage", pkm.stat_defense)
      .Add("SpA Stage", pkm.stat_special_attack)
      .Add("SpD Stage", pkm.stat_special_defense)
      .Add("Spe Stage", pkm.stat_speed)
      .Add("Acc Stage", pkm.stat_accuracy)
      .Add("Eva Stage", pkm.stat_evasion)

      .Add("EV Total", pkm.ev_sum)
      .Add("EV HP", pkm.ev_hp)
      .Add("EV Atk", pkm.ev_attack)
      .Add("EV Def", pkm.ev_defense)
      .Add("EV Spe", pkm.ev_speed)
      .Add("EV SpA", pkm.ev_special_attack)
      .Add("EV SpD", pkm.ev_special_defense)

      .AddType("Type 1", pkm.types[0])
      .AddType("Type 2", pkm.types[1])
      .AddType("Type 3", pkm.types[2])

      .Add("Level", pkm.level)
      .Add("Experience", pkm.experience)
      .Add("Friendship", pkm.friendship)
      .Add("Form", pkm.form)
      .Add("Gender", pkm.gender)

      .AddAbility("Ability", pkm.ability)
      .Add("Item", pkm.item)
      .Add("Weight", pkm.weight);
}

u32 OnStartTurn(uptr server, u32 action) {
  // uptr server = READ(u32, 0x082061F8);

  // Change Ability
  {
    struct {
      u32 flags;
      u16 ability;
      u8 id;
      u8 _0;
      u8 _1;
      u32 message[9];
    } cmd{};
    cmd.id = 0;

    cmd.ability = 191;
    ((void (*)(uptr, void*))0x07184D8)(server, &cmd);

    cmd.ability = 189;
    ((void (*)(uptr, void*))0x07184D8)(server, &cmd);
  }

  // Change HP
  {
    struct {
      u32 flags;
      u8 count;
      u8 _0[2];
      u8 id[10];
      s32 hp[10];
    } cmd{};

    cmd.id[0] = 0;
    cmd.count = 1;

    cmd.hp[0] = -200;
    ((void (*)(uptr, void*))0x007135B0)(server, &cmd);

    cmd.hp[0] = 150;
    ((void (*)(uptr, void*))0x007135B0)(server, &cmd);
  }

  // Kill
  {
    struct {
      u32 flags;
      u8 id;
      u8 _0;
      u16 _1;
      u32 message[9];
    } cmd{};

    cmd.id = 0;
    ((void (*)(uptr, void*))0x0710D3C)(server, &cmd);
  }

  // Revive
  {
    struct {
      u32 flags;
      u8 id;
      u16 hp;
      u32 message[9];
    } cmd{};

    cmd.id = 0;
    cmd.hp = 999;

    ((void (*)(uptr, void*))0x00711F08)(server, &cmd);
  }

  // Metamorph
  {
    struct {
      u32 flags : 8;
      u32 source : 5;
      u32 : 19;
      u8 target;
      u32 msg[9];
    } cmd{};

    cmd.source = 0;
    cmd.target = 12;

    ((void (*)(uptr, void*))0x00711CE4)(server, &cmd);
  }

  // Attack
  {
    struct {
      u32 action : 4;
      u32 target : 4;
      u32 move : 16;
    } cmd{};

    cmd.action = 1;  // attack
    cmd.target = 1;  // enemy
    cmd.move = 588;

    ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
        server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);

    cmd.move = 604;
    ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
        server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);

    cmd.move = 617;
    ((void (*)(uptr, Pokemon*, void*, u32, u32))0x0070EC48)(
        server, Manager::GetPokemon(true, 0, 0), &cmd, 0, 0);
  }

  return HookManager::GetInstance()
      .Get(HookID::kOnStartTurn)
      ->CallOriginal<u32>(server, action);
}

void PlayBattleAnimation(void* graphics, u16 animation) {
  // shiny anim -> rainbow anim
  if (animation == 621) {
    animation = 635;
  }
  HookManager::GetInstance()
      .Get(HookID::kPlayBattleAnimation)
      ->CallOriginal<void>(graphics, animation);
}

void PokemonModel::LoadMenu(menu::PluginMenu& menu, void* args) {
  PokemonModel& model =
      Manager::GetInstance().GetGraphics().GetPokemonModel(ctx.pokemon_idx);

  model.update = true;

  constexpr f32 kScaleFactor = 0.05f;
  constexpr f32 kRotationFactor = 4.0f * M_PI / 180.0f;
  constexpr f32 kPositionFactor = 4.0f;

  menu.Add("Position X", model.position.x)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .Add("Position Y", model.position.y)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .Add("Position Z", model.position.z)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .Add("Rotation X", model.rotation.x)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .Add("Rotation Y", model.rotation.y)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .Add("Rotation Z", model.rotation.z)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .Add("Scale X", model.scale.x)
      .WithFactor(kScaleFactor)
      .WithRefresh()
      .Add("Scale Y", model.scale.y)
      .WithFactor(kScaleFactor)
      .WithRefresh()
      .Add("Scale Z", model.scale.z)
      .WithFactor(kScaleFactor)
      .WithRefresh();
}

void Manager::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager::GetInstance().Add(HookID::kOnStartTurn, 0x00759B74,
                                 (uptr)OnStartTurn);

  HookManager::GetInstance().Add(HookID::kPlayBattleAnimation, 0x007510A8,
                                 (uptr)PlayBattleAnimation);

  ctx.pkm_server = GetPokemon(true, ctx.team_idx, ctx.pokemon_idx);
  ctx.pkm_client = GetPokemon(false, ctx.team_idx, ctx.pokemon_idx);

  menu.Add("Team Index", ctx.team_idx)
      .WithRefresh()
      .Add("Pokemon Index", ctx.pokemon_idx)
      .WithRefresh()
      .Add("Pokemon Data", Pokemon::LoadMenu)
      .Add("Pokemon Model", PokemonModel::LoadMenu);
}

}  // namespace battle