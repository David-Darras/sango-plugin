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
 * @brief The features that Sango Studio turns on or off.
 *
 * The declarations are in core/patch/studio_features.h.
 */

#include "core/patch/studio_features.h"

#include "battle/patch/shiny_entry.h"
#include "battle/patch/trainer_model_extension.h"
#include "core/ini.h"
#include "core/patch/music_files.h"
#include "core/patch/music_lock.h"
#include "overworld/patch/auto_surf.h"
#include "ui/patch/app_status.h"
#include "ui/patch/title_screen.h"
#include "ui/performance_overlay.h"
#ifdef GAME_ORAS
#include "net/remote_avatars.h"
#include "overworld/patch/healer_follower.h"
#include "overworld/patch/map_editor.h"
#include "overworld/patch/map_graft.h"
#include "overworld/patch/pokemon_flight.h"
#include "overworld/patch/run_animation.h"
#include "overworld/patch/tall_grass.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/world_overlay.h"
#endif

namespace core {
namespace {
const c16* const kPath = u"sdmc:/sango-studio/features.ini";
const c8* const kSection = "features";

// The switch of each feature. A switch is asked only when the file has a line
// for it: some features are made at their first use.
struct Feature {
  const c8* key; ///< The key of the file: Sango Studio uses the same names.
  bool& (*get)();
};

#define SANGO_FEATURE(key, expression) \
  { key, []() -> bool& { return expression; } }

const Feature kFeatures[] = {
    SANGO_FEATURE("title_screen", ui::TitleScreen::GetInstance().is_enabled),
    SANGO_FEATURE("trainer_models",
                  battle::TrainerModelExtension::GetInstance().is_enabled),
    SANGO_FEATURE("music_files", MusicFiles::GetInstance().is_enabled),
    SANGO_FEATURE("music_lock", MusicLock::GetInstance().is_enabled),
    SANGO_FEATURE("shiny_entry", battle::ShinyEntry::GetInstance().is_enabled),
    SANGO_FEATURE("summary_editor", ui::AppStatus::GetInstance().is_enabled),
    SANGO_FEATURE("auto_surf", overworld::AutoSurf::GetInstance().is_enabled),
    SANGO_FEATURE("performance",
                  ui::PerformanceOverlay::GetInstance().is_enabled),
#ifdef GAME_ORAS
    SANGO_FEATURE("run_animation",
                  overworld::RunAnimation::GetInstance().enabled),
    SANGO_FEATURE("other_players",
                  net::RemoteAvatars::GetInstance().is_enabled),
    SANGO_FEATURE("nurse_follower",
                  overworld::HealerFollower::GetInstance().is_enabled),
    SANGO_FEATURE("pokemon_flight",
                  overworld::PokemonFlight::GetInstance().is_enabled),
    SANGO_FEATURE("tall_grass", overworld::TallGrass::GetInstance().is_enabled),
    SANGO_FEATURE("tile_changes",
                  overworld::TileEditor::GetInstance().is_enabled),
    SANGO_FEATURE("map_editor_changes",
                  overworld::MapEditor::GetInstance().is_enabled),
    SANGO_FEATURE("joined_maps", overworld::MapGraft::GetInstance().is_enabled),
    SANGO_FEATURE("map_marks",
                  overworld::WorldOverlay::GetInstance().is_enabled),
#endif
};

#undef SANGO_FEATURE
} // namespace

void StudioFeatures::Apply() {
  // 8 KB: out of the stack.
  static Ini ini;
  if (!ini.Load(kPath)) return;
  for (const Feature& feature : kFeatures) {
    if (ini.Get(kSection, feature.key) == nullptr) continue;
    bool& is_enabled = feature.get();
    is_enabled = ini.GetBool(kSection, feature.key, is_enabled);
  }
}
} // namespace core
