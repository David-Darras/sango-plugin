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

#include <cstring>

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
    memcpy(ctx.pkm_server->moves[i].core, ctx.pkm_server->moves[i].view,
           sizeof(ctx.pkm_server->moves[i].view));
  }
  memcpy(ctx.pkm_client, ctx.pkm_server, sizeof(Pokemon));
}

void Pokemon::LoadMenu(menu::PluginMenu& menu, void* args) {
  Pokemon& pkm = *ctx.pkm_server;

  menu.Add("Save", OnSave)
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

void Manager::LoadMenu(menu::PluginMenu& menu, void* args) {
  ctx.pkm_server = GetPokemon(true, ctx.team_idx, ctx.pokemon_idx);
  ctx.pkm_client = GetPokemon(false, ctx.team_idx, ctx.pokemon_idx);

  menu.Add("Team Index", ctx.team_idx)
      .WithRefresh()
      .Add("Pokemon Index", ctx.pokemon_idx)
      .WithRefresh()
      .Add("Pokemon Data", Pokemon::LoadMenu);
}

}  // namespace battle