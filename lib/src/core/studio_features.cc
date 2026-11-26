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
 * @file studio_features.cc
 * @brief The features that Sango Studio sets.
 *
 * The declarations are in core/patch/studio_features.h.
 *
 * Each setting is one line of the table: its key in the file and the variable.
 * The Features page of Sango Studio has the same keys.
 */

#include "core/patch/studio_features.h"

#include <type_traits>

#include "battle/patch/battle.h"
#include "battle/patch/setup.h"
#include "battle/patch/shiny_entry.h"
#include "battle/patch/trainer_model_extension.h"
#include "core/cheat_code_manager.h"
#include "core/ini.h"
#include "core/parse_float.h"
#include "core/patch/game_speed.h"
#include "core/patch/music_files.h"
#include "core/patch/music_lock.h"
#include "core/patch/time_override.h"
#include "overworld/patch/auto_surf.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/weather_override.h"
#include "pokemon/patch/mega_evolution.h"
#include "pokemon/patch/shiny.h"
#include "renderer/patch/lighting.h"
#include "renderer/patch/model_filter.h"
#include "ui/patch/app_status.h"
#include "ui/patch/keyboard_patch.h"
#include "ui/patch/new_game.h"
#include "ui/patch/title_screen.h"
#include "ui/performance_overlay.h"
#ifdef GAME_ORAS
#include "net/remote_avatars.h"
#include "overworld/patch/field_move.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/healer_follower.h"
#include "overworld/patch/map_character.h"
#include "overworld/patch/map_editor.h"
#include "overworld/patch/map_graft.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/pokemon_flight.h"
#include "overworld/patch/run_animation.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/tall_grass.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/trade.h"
#include "overworld/patch/world_overlay.h"
#include "pokemon/patch/item_customizer.h"
#include "ui/page/pages.h"
#endif

namespace core {
namespace {
const c16* const kPath = u"sdmc:/sango-studio/features.ini";
const c8* const kSection = "features";

// The value of a line, in the type of the variable.
void Read(const c8* text, bool& value) {
  const c8 first = text[0] | 0x20;
  // 1, on, true and yes are true.
  value = text[0] == '1' || first == 't' || first == 'y' ||
          (first == 'o' && (text[1] | 0x20) == 'n');
}

void Read(const c8* text, f32& value) {
  c8* end = nullptr;
  const f32 read = ParseFloat(text, &end);
  if (end != text) value = read;
}

// A color: RRGGBB in hexadecimal. The alpha stays.
void Read(const c8* text, Color& value) {
  u32 hex = 0;
  u32 count = 0;
  for (; text[count] != '\0' && count < 6; count++) {
    const c8 digit = text[count] | 0x20;
    if (digit >= '0' && digit <= '9') {
      hex = hex * 16 + (digit - '0');
    } else if (digit >= 'a' && digit <= 'f') {
      hex = hex * 16 + (digit - 'a' + 10);
    } else {
      return;
    }
  }
  if (count != 6) return;
  value.r = ((hex >> 16) & 0xFF) / 255.0f;
  value.g = ((hex >> 8) & 0xFF) / 255.0f;
  value.b = (hex & 0xFF) / 255.0f;
}

// A whole number: a number, or a value of an enum.
template <class T>
typename std::enable_if<std::is_integral<T>::value ||
                        std::is_enum<T>::value>::type
Read(const c8* text, T& value) {
  const bool is_negative = text[0] == '-';
  u32 at = is_negative ? 1 : 0;
  if (text[at] < '0' || text[at] > '9') return;
  s32 number = 0;
  for (; text[at] >= '0' && text[at] <= '9'; at++) {
    number = number * 10 + (text[at] - '0');
  }
  value = static_cast<T>(is_negative ? -number : number);
}

// The lines of a setting. The variable is asked only when the file has the
// line: some features are made at their first use.
struct Setting {
  const c8* key;
  void (*set)(const c8* text);
};

#define SANGO_FEATURE(key, expression) \
  { key, [](const c8* text) { Read(text, expression); } }

const Setting kSettings[] = {
    // Graphics.
    SANGO_FEATURE("graphics.outlines",
                  renderer::Lighting::GetInstance().use_outline),
    SANGO_FEATURE("graphics.outline_width",
                  renderer::Lighting::GetInstance().outline_scale),
    SANGO_FEATURE("graphics.use_outline_color",
                  renderer::Lighting::GetInstance().use_outline_color),
    SANGO_FEATURE("graphics.outline_color",
                  renderer::Lighting::GetInstance().outline_color),
    SANGO_FEATURE("graphics.use_ambient_light",
                  renderer::Lighting::GetInstance().use_ambient_light),
    SANGO_FEATURE("graphics.ambient_color",
                  renderer::Lighting::GetInstance().ambient_color),
    SANGO_FEATURE("graphics.use_diffuse_light",
                  renderer::Lighting::GetInstance().use_diffuse_light),
    SANGO_FEATURE("graphics.diffuse_color",
                  renderer::Lighting::GetInstance().diffuse_color),
    SANGO_FEATURE("graphics.pokemon_filter",
                  renderer::ModelFilter::GetInstance().filter),
    // Title screen.
    SANGO_FEATURE("title.top_video",
                  ui::TitleScreen::GetInstance().top_video),
    SANGO_FEATURE("title.bottom_video",
                  ui::TitleScreen::GetInstance().bottom_video),
    SANGO_FEATURE("title.cry_species",
                  ui::TitleScreen::GetInstance().pokemon_cry_species),
    SANGO_FEATURE("title.cry_volume",
                  ui::TitleScreen::GetInstance().pokemon_cry_volume),
    SANGO_FEATURE("title.no_delay", ui::TitleScreen::GetInstance().no_delay),
    SANGO_FEATURE("title.no_shadow", ui::TitleScreen::GetInstance().no_shadow),
    SANGO_FEATURE("title.skip_to_frame",
                  ui::TitleScreen::GetInstance().skip_to_frame),
    // New game.
    SANGO_FEATURE("new_game.skip_intro",
                  ui::NewGame::GetInstance().skip_intro),
    SANGO_FEATURE("new_game.player_gender",
                  ui::NewGame::GetInstance().player_gender),
    // Battle.
    SANGO_FEATURE("battle.exp_multiplier",
                  battle::Battle::GetInstance().exp_multiplier),
    SANGO_FEATURE("battle.catch_multiplier",
                  battle::Battle::GetInstance().catch_multiplier),
    SANGO_FEATURE("battle.money_multiplier",
                  battle::Battle::GetInstance().money_multiplier),
    SANGO_FEATURE("battle.same_catch_rate",
                  battle::Battle::GetInstance().same_ratio_for_all_pokeball),
    SANGO_FEATURE("battle.inverse_stats",
                  battle::Battle::GetInstance().inverse_stats),
    SANGO_FEATURE("battle.metronome_only",
                  battle::Battle::GetInstance().metronome_only),
    SANGO_FEATURE("battle.shared_team_hp",
                  battle::Battle::GetInstance().sync_team_hp),
    SANGO_FEATURE("battle.can_use_items",
                  battle::Battle::GetInstance().can_use_item),
    SANGO_FEATURE("battle.one_mega_evolution",
                  battle::Battle::GetInstance().mega_restriction),
    SANGO_FEATURE("battle.unlimited_mega_evolution",
                  battle::Battle::GetInstance().unlimited_mega_evolution),
    SANGO_FEATURE("battle.real_sizes",
                  battle::Battle::GetInstance().fix_pokemon_size),
    SANGO_FEATURE("battle.type_helper",
                  battle::Battle::GetInstance().show_type_helper),
    SANGO_FEATURE("battle.hp_gradient",
                  battle::Battle::GetInstance().hp_gradient),
    SANGO_FEATURE("battle.no_shader", battle::Battle::GetInstance().no_shader),
    SANGO_FEATURE("battle.fixed_camera",
                  battle::Battle::GetInstance().fixed_camera),
    SANGO_FEATURE("battle.encounter_animation",
                  battle::Battle::GetInstance().encounter_animation),
    SANGO_FEATURE("battle.mega_evolve_animation",
                  battle::Battle::GetInstance().mega_evolve_animation),
    SANGO_FEATURE("battle.enemy_view",
                  battle::Battle::GetInstance().show_enemy_pov),
    SANGO_FEATURE("battle.trainer_animation",
                  battle::Battle::GetInstance().show_trainer_animation),
    SANGO_FEATURE("battle.ball_animation",
                  battle::Battle::GetInstance().show_pokeball_animation),
    SANGO_FEATURE("battle.fade_in",
                  battle::Battle::GetInstance().show_fade_in),
    SANGO_FEATURE("battle.shiny_animation",
                  battle::Battle::GetInstance().show_shiny_animation),
    SANGO_FEATURE("battle.shiny_entry",
                  battle::ShinyEntry::GetInstance().is_enabled),
    SANGO_FEATURE("battle.trainer_models",
                  battle::TrainerModelExtension::GetInstance().is_enabled),
    SANGO_FEATURE("battle.player_model",
                  battle::TrainerModelExtension::GetInstance().player_model),
    // Wild battles.
    SANGO_FEATURE("wild.enabled", battle::Setup::GetInstance().is_enabled),
    SANGO_FEATURE("wild.species", battle::Setup::GetInstance().species),
    SANGO_FEATURE("wild.form", battle::Setup::GetInstance().form),
    SANGO_FEATURE("wild.format", battle::Setup::GetInstance().format),
    SANGO_FEATURE("wild.inverse_teams",
                  battle::Setup::GetInstance().inverse_teams),
    // Pokémon.
    SANGO_FEATURE("pokemon.shiny_rate", pokemon::Shiny::GetInstance().rate),
    SANGO_FEATURE("pokemon.mega_mime_jr",
                  pokemon::MegaEvolution::GetInstance().has_mega_mime_jr),
    SANGO_FEATURE("pokemon.summary_editor",
                  ui::AppStatus::GetInstance().is_enabled),
    SANGO_FEATURE("pokemon.instant_egg_hatch",
                  overworld::DayCare::GetInstance().instant_egg_hatch),
    SANGO_FEATURE("pokemon.instant_max_exp",
                  overworld::DayCare::GetInstance().instant_max_exp),
    // Overworld.
    SANGO_FEATURE("overworld.auto_surf",
                  overworld::AutoSurf::GetInstance().is_enabled),
    SANGO_FEATURE("weather.keep_indoors",
                  overworld::WeatherOverride::GetInstance()
                      .keep_weather_indoors),
    SANGO_FEATURE("weather.ignore_zone",
                  overworld::WeatherOverride::GetInstance()
                      .ignore_zone_weather),
    SANGO_FEATURE("weather.rain_color",
                  overworld::WeatherOverride::GetInstance().mode),
    SANGO_FEATURE("time.enabled", TimeOverride::GetInstance().is_enabled),
    SANGO_FEATURE("time.hour", TimeOverride::GetInstance().hour),
    SANGO_FEATURE("time.minute", TimeOverride::GetInstance().minute),
    // System.
    SANGO_FEATURE("system.game_speed", GameSpeed::GetInstance().game_speed),
    SANGO_FEATURE("system.frame_rate", GameSpeed::GetInstance().frame_rate),
    SANGO_FEATURE("system.music_files",
                  MusicFiles::GetInstance().is_enabled),
    SANGO_FEATURE("system.music_lock", MusicLock::GetInstance().is_enabled),
    SANGO_FEATURE("system.free_keyboard",
                  ui::KeyboardPatch::GetInstance().is_enabled),
    SANGO_FEATURE("system.performance",
                  ui::PerformanceOverlay::GetInstance().is_enabled),
#ifdef GAME_ORAS
    SANGO_FEATURE("overworld.run_animation",
                  overworld::RunAnimation::GetInstance().enabled),
    SANGO_FEATURE("overworld.fly_anywhere",
                  overworld::FieldMove::GetInstance().fly_anywhere),
    SANGO_FEATURE("overworld.no_collision",
                  overworld::PlayerCheats::GetInstance().no_collision),
    SANGO_FEATURE("overworld.walk_frames",
                  overworld::PlayerCheats::GetInstance().walk_frames),
    SANGO_FEATURE("overworld.run_frames",
                  overworld::PlayerCheats::GetInstance().run_frames),
    SANGO_FEATURE("overworld.other_players",
                  net::RemoteAvatars::GetInstance().is_enabled),
    SANGO_FEATURE("overworld.following_pokemon", FollowingPokemonSwitch()),
    SANGO_FEATURE("overworld.nurse_follower",
                  overworld::HealerFollower::GetInstance().is_enabled),
    SANGO_FEATURE("overworld.example_characters",
                  overworld::MapCharacter::GetInstance().show_examples),
    SANGO_FEATURE("flight.enabled",
                  overworld::PokemonFlight::GetInstance().is_enabled),
    SANGO_FEATURE("flight.species",
                  overworld::PokemonFlight::GetInstance().species),
    SANGO_FEATURE("flight.size",
                  overworld::PokemonFlight::GetInstance().scale),
    SANGO_FEATURE("flight.speed",
                  overworld::PokemonFlight::GetInstance().speed),
    SANGO_FEATURE("flight.climb_speed",
                  overworld::PokemonFlight::GetInstance().climb_speed),
    SANGO_FEATURE("ride.enabled",
                  overworld::PokemonFlight::GetInstance().is_ride_enabled),
    SANGO_FEATURE("ride.species",
                  overworld::PokemonFlight::GetInstance().ride_species),
    SANGO_FEATURE("ride.size",
                  overworld::PokemonFlight::GetInstance().ride_scale),
    SANGO_FEATURE("ride.speed",
                  overworld::PokemonFlight::GetInstance().ride_speed),
    SANGO_FEATURE("ride.height",
                  overworld::PokemonFlight::GetInstance().ride_height),
    SANGO_FEATURE("ride.spread",
                  overworld::PokemonFlight::GetInstance().ride_spread),
    SANGO_FEATURE("ride.back",
                  overworld::PokemonFlight::GetInstance().ride_back),
    SANGO_FEATURE("grass.enabled",
                  overworld::TallGrass::GetInstance().is_enabled),
    SANGO_FEATURE("grass.types", overworld::TallGrass::GetInstance().mix),
    SANGO_FEATURE("grass.radius", overworld::TallGrass::GetInstance().radius),
    SANGO_FEATURE("grass.density",
                  overworld::TallGrass::GetInstance().density),
    SANGO_FEATURE("marks.enabled",
                  overworld::WorldOverlay::GetInstance().is_enabled),
    SANGO_FEATURE("marks.tiles",
                  overworld::WorldOverlay::GetInstance().show_tiles),
    SANGO_FEATURE("marks.signs",
                  overworld::WorldOverlay::GetInstance().show_signs),
    SANGO_FEATURE("marks.triggers",
                  overworld::WorldOverlay::GetInstance().show_triggers),
    SANGO_FEATURE("marks.warps",
                  overworld::WorldOverlay::GetInstance().show_warps),
    SANGO_FEATURE("marks.hidden_items",
                  overworld::WorldOverlay::GetInstance().show_hidden_items),
    SANGO_FEATURE("marks.characters",
                  overworld::WorldOverlay::GetInstance().show_characters),
    SANGO_FEATURE("marks.trainer_sight",
                  overworld::WorldOverlay::GetInstance().show_trainer_sight),
    SANGO_FEATURE("marks.legend",
                  overworld::WorldOverlay::GetInstance().show_legend),
    SANGO_FEATURE("marks.radius",
                  overworld::WorldOverlay::GetInstance().radius),
    SANGO_FEATURE("marks.opacity",
                  overworld::WorldOverlay::GetInstance().opacity),
    SANGO_FEATURE("maps.editor_changes",
                  overworld::MapEditor::GetInstance().is_enabled),
    SANGO_FEATURE("maps.tile_changes",
                  overworld::TileEditor::GetInstance().is_enabled),
    SANGO_FEATURE("maps.joined_maps",
                  overworld::MapGraft::GetInstance().is_enabled),
    SANGO_FEATURE("pokemon.random_gifts",
                  overworld::GiftPokemon::GetInstance().randomize_species),
    SANGO_FEATURE("pokemon.random_trades",
                  overworld::Trade::GetInstance().randomize_species),
    SANGO_FEATURE("pokemon.random_encounters",
                  overworld::StaticRandomizer::GetInstance().randomize_species),
    SANGO_FEATURE("pokemon.no_ev_limit",
                  pokemon::ItemCustomizer::GetInstance().remove_limit),
#endif
};

#undef SANGO_FEATURE

// A cheat code of the menu, which the file turns on.
void ApplyCheatCode(const Ini& ini, const c8* key, CheatCodeId id) {
  const c8* text = ini.Get(kSection, key);
  if (text == nullptr) return;
  bool wanted = false;
  Read(text, wanted);
  CheatCode* code = CheatCodeManager::GetInstance().Get(id);
  if (code != nullptr && code->IsEnabled() != wanted) code->Toggle();
}
} // namespace

void StudioFeatures::Apply() {
  // 8 KB: out of the stack.
  static Ini ini;
  if (!ini.Load(kPath)) return;
  for (const Setting& setting : kSettings) {
    const c8* text = ini.Get(kSection, setting.key);
    if (text != nullptr) setting.set(text);
  }
  ApplyCheatCode(ini, "overworld.noclip", CheatCodeId::kNoclip);
  ApplyCheatCode(ini, "overworld.swarm", CheatCodeId::kSwarmMod);
#ifdef GAME_ORAS
  pokemon::ItemCustomizer::UpdateLimitPatch();
#endif
}
} // namespace core
