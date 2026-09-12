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

#include "feature/battle/feature_battle.h"
#include "game/constant/event.h"
#include "game/overworld/map_data.h"
#include "game/savedata/event_table.h"
#include "game/savedata/misc.h"
#include "game/savedata/pokemon_utils.h"
#include "game/savedata/settings.h"

namespace kaizo {
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

void ApplyLevelCaps(battle::Team* team,
                    void* raw_data) {
  auto* data = (feature::Battle::LevelUpData*)raw_data;
  u32 count = savedata::Misc::GetInstance().GetBadgesCount();
  u32 max_level = LEVEL_CAPS[count];

  if (count == 0) {
    auto& event = savedata::EventTable::GetInstance();
    if (event.Check(EventID::kRoute103Unlocked)) {
      max_level = 8;
    }
    if (event.Check(EventID::kRoute102Unlocked)) {
      max_level = 11;
    }
  }

  for (u32 i = 0; i < team->count; i++) {
    data[i].ev_hp = 0;
    data[i].ev_attack = 0;
    data[i].ev_defense = 0;
    data[i].ev_speed = 0;
    data[i].ev_special_attack = 0;
    data[i].ev_special_defense = 0;

    // Game finished => Max Level = 100
    if (!savedata::EventTable::GetInstance().Check(EventID::kGameFinished)) {
      u8 new_level = PokemonUtils::GetLevelFromExperience(
          team->pokemon[i]->species,
          team->pokemon[i]->form,
          team->pokemon[i]->experience + data[i].exp);
      if (new_level >= max_level) {
        data[i].exp = 0;
      }
    }
  }
}

void PatchBattle() {
  auto& battle = feature::Battle::GetInstance();
  battle.can_use_item = false;
  battle.fix_pokemon_size = true;
  battle.no_shader = false;
  battle.mega_restriction = false;
  battle.same_ratio_for_all_pokeball = true;
  battle.is_long_encounter_animation = false;
  battle.is_long_mega_evolve_animation = true;
  battle.show_enemy_pov = false;
  battle.show_fade_in = true;
  battle.show_pokeball_animation = false;
  battle.show_shiny_animation = false;
  battle.show_trainer_animation = false;
  battle.sync_overworld_music = true;

  savedata::Settings::GetInstance().battle_background_id =
      Utils::GetRandomValue(15);
}
}