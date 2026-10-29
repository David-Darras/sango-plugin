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
 * @file page_cheats.cc
 * @brief The menu page of the cheats: fun features that do not change a
 *        structure of the game.
 */

#include "overworld/patch/auto_surf.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/healer_follower.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/run_animation.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
#ifdef GAME_ORAS
static const c8* kStepFrames[] = {"1", "2", "3", "4", "5",
                                  "6", "7", "8", "12"};
#endif

void LoadCheatsPage(MainApplication& app, void* args) {
  auto& swarm = overworld::PlayerCheats::GetInstance();
  auto& day_care = overworld::DayCare::GetInstance();

  app.AddSection("Movement")
     .Add("Noclip", CheatCodeId::kNoclip)
     .WithDescription("The player walks through the walls. L and R: down and "
                      "up.")
#ifdef GAME_ORAS
     .Add("No Collision", swarm.no_collision)
     .WithDescription("The player walks through the walls, the objects and "
                      "the steps of the ground, with the normal walk. The "
                      "map is not changed.")
#endif
#ifdef GAME_ORAS
     .Add("Walk Frames Per Tile", swarm.walk_frames)
     .WithArray(kStepFrames, SIZE(kStepFrames))
     .WithDescription("The frames that the player needs to walk one tile. "
                      "Fewer frames is faster. The game uses 7.")
     .Add("Run Frames Per Tile", swarm.run_frames)
     .WithArray(kStepFrames, SIZE(kStepFrames))
     .WithDescription("The same, when the player runs. The game uses 4.")
#endif
     .Add("Auto Surf", overworld::AutoSurf::GetInstance().is_enabled)
     .WithDescription("The player surfs at once when walking into water.")
     .Add("Run Animation", overworld::RunAnimation::GetInstance().enabled)
     .WithDescription("The characters that have no run animation of their "
                      "own use a faster walk animation when they run.")
     .AddSection("Followers")
     .Add("Swarm Mod", CheatCodeId::kSwarmMod)
     .WithDescription("The people of the map follow the player in a circle.")
     .Add("Swarm Radius", swarm.radius)
     .Add("Swarm Rotation Speed", swarm.theta_speed)
#ifdef GAME_ORAS
     .Add("Healer Follower", overworld::HealerFollower::GetInstance().is_enabled)
     .WithDescription("A nurse follows the player. Talk to her to heal the "
                      "team.")
#endif
     .AddSection("Day Care")
     .Add("Instant Egg Hatch", day_care.instant_egg_hatch)
     .WithCallback(overworld::DayCare::ApplyEggHatch)
     .WithDescription("Press {A} to switch it. The Eggs hatch at once.")
     .Add("Instant Max Exp", day_care.instant_max_exp)
     .WithCallback(overworld::DayCare::ApplyMaxExp)
     .WithDescription("Press {A} to switch it. The Pokemon of the Day Care get "
                      "the maximum experience.")
     ;
}
} // namespace ui
