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

#include "savedata/pokemon_core_data.h"

#include <cstring>

#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
#include "savedata/pokemon_data_accessor.h"
#include "savedata/pokemon_utils.h"

static struct {
  u8 level;
  bool is_shiny;
  PokemonDataAccessor accessor;
  PokemonCoreData backup_core_data;
  PokemonCoreData* core_data;
} ctx;

void Save(void*) {
  PokemonCoreData* pkm = ctx.accessor.GetCoreData();

  pkm->experience =
      PokemonUtils::GetExperienceFromLevel(pkm->species, pkm->form, ctx.level);
  if (ctx.is_shiny) {
    PokemonUtils::ConvertToShiny(pkm->id, &pkm->shiny_id);
  } else {
    PokemonUtils::ConvertToNormal(pkm->id, &pkm->shiny_id);
  }

  ctx.accessor.Encrypt();
  std::memcpy(ctx.core_data, &ctx.backup_core_data,
              sizeof(ctx.backup_core_data));
  ctx.accessor.Decrypt();
}

void PokemonCoreData::LoadMenu(menu::PluginMenu& menu, void* args) {
  ctx.core_data = (PokemonCoreData*)args;
  std::memcpy(&ctx.backup_core_data, ctx.core_data,
              sizeof(ctx.backup_core_data));

  ctx.accessor.Initialize(&ctx.backup_core_data, nullptr);
  ctx.accessor.Decrypt();

  PokemonCoreData* pkm = ctx.accessor.GetCoreData();

  ctx.level = PokemonUtils::GetLevelFromExperience(pkm->species, pkm->form,
                                                   pkm->experience);
  ctx.is_shiny = PokemonUtils::IsShiny(pkm->id, pkm->shiny_id);

  menu.Add("Save", Save)
      .Add("Species", pkm->species)
      .Add("Is Shiny", ctx.is_shiny)
      .Add("Nickname", pkm->nickname, 13)
      .Add("Gender", &pkm->event_gender_form_flags, 1, 2)
      .Add("Form", &pkm->event_gender_form_flags, 3, 5)
      .Add("Level", ctx.level)
      .Add("Nature", pkm->nature)
      .Add("Ability", pkm->ability)
      .Add("Item", pkm->item)
      .Add("Ball", pkm->ball)
      .Add("O.T", pkm->original_trainer_name, 13)
      .Add("Language", pkm->language)
      .Add("In egg", &pkm->iv_flags, 30, 1);

  menu.Add("Move 1", pkm->moves[0])
      .Add("Move 2", pkm->moves[1])
      .Add("Move 3", pkm->moves[2])
      .Add("Move 4", pkm->moves[3]);

  menu.Add("PP 1", pkm->pp[0])
      .Add("PP 2", pkm->pp[1])
      .Add("PP 3", pkm->pp[2])
      .Add("PP 4", pkm->pp[3]);

  menu.Add("EV - HP", pkm->ev.hp)
      .Add("EV - Attack", pkm->ev.attack)
      .Add("EV - Defense", pkm->ev.defense)
      .Add("EV - Speed", pkm->ev.speed)
      .Add("EV - Special Attack", pkm->ev.special_attack)
      .Add("EV - Special Defense", pkm->ev.special_defense);

  menu.Add("IV - HP", &pkm->iv_flags, 0, 5)
      .Add("IV - Attack", &pkm->iv_flags, 5, 5)
      .Add("IV - Defense", &pkm->iv_flags, 10, 5)
      .Add("IV - Speed", &pkm->iv_flags, 15, 5)
      .Add("IV - Special Attack", &pkm->iv_flags, 20, 5)
      .Add("IV - Special Defense", &pkm->iv_flags, 25, 5);

  menu.Add("Contest - Cool", pkm->contest.cool)
      .Add("Contest - Beautiful", pkm->contest.beautiful)
      .Add("Contest - Cute", pkm->contest.cute)
      .Add("Contest - Smart", pkm->contest.smart)
      .Add("Contest - Tough", pkm->contest.tough)
      .Add("Contest - Sheen", pkm->contest.sheen);
}
