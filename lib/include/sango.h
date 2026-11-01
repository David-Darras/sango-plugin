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
 * @file sango.h
 * @brief One header for a ROM hack: it includes the features of the library.
 *
 * Include this file in each file of your ROM hack. You then do not have to
 * find the header of each feature.
 *
 * The file does not include the game structures of each domain (the `native`
 * folders), the addresses (`<domain>/address.h`) and the tools of the menu
 * pages. Include them yourself when you need them, for example
 * `pokemon/address.h` for a HOOK().
 */

#pragma once

#include "common.h"
#include "hack.h"
#include "plugin.h"

// The ids: species, moves, items, maps, flags...
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/format.h"
#include "battle/constant/ground.h"
#include "battle/constant/moment_kind.h"
#include "battle/constant/platform.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/trainer.h"
#include "battle/constant/type_multiplier.h"
#include "battle/constant/weather.h"
#include "core/constant/event_flag.h"
#include "overworld/constant/encounter_method.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/ball.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/shiny_rate.h"
#include "pokemon/constant/type.h"
#include "script/constant/script.h"

// The data of the game that a ROM hack changes.
#include "battle/native/controller.h"
#include "overworld/native/wild_pokemon.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"

// Battle: new moves, abilities, types and items, trainer teams, animations.
#include "battle/patch/battle.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/item_extension.h"
#include "battle/patch/move_animation.h"
#include "battle/patch/setup.h"
#include "battle/patch/trainer_team.h"
#include "battle/patch/type_chart.h"

// Pokémon: species data, shiny rate, Mega Evolutions, shops, items, models.
#include "pokemon/edit_pokemon.h"
#include "pokemon/patch/species_editor.h"
#include "pokemon/patch/custom_shop.h"
#include "pokemon/patch/item_customizer.h"
#include "pokemon/patch/mega_evolution.h"
#include "pokemon/patch/model_replacement.h"
#include "pokemon/patch/shiny.h"

// Overworld: wild Pokémon, characters, weather, camera.
#include "overworld/patch/camera.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/overworld.h"
#include "overworld/patch/weather_override.h"
#include "overworld/patch/wild_encounter.h"

// Save data: the flags and the variables of the ROM hack.
#include "savedata/patch/hack_save.h"

// Scripts: the C++ scripts of the characters.
#include "script/patch/context.h"
#include "script/patch/native_script.h"

// Tools: translations, hooks, random numbers, music, log.
#include "core/translations.h"
#include "core/hook.h"
#include "core/patch/music_lock.h"
#include "core/utils.h"
#include "ui/log.h"

// The menu.
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/painter.h"
