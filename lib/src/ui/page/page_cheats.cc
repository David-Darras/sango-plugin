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
#include "overworld/patch/pokemon_flight.h"
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
     .Add("Following Pokemon", FollowingPokemonSwitch())
     .WithCallback(ApplyFollowingPokemon)
     .WithDescription("The first Pokemon of the team walks near the player. "
                      "L plays a battle animation and R a Pokemon-Amie "
                      "animation, with its cry. It leaves during the "
                      "menus, the scripts and the battles.")
#endif
#ifdef GAME_ORAS
     .AddSection("Flight")
     .Add("Fly On A Pokemon", overworld::PokemonFlight::GetInstance().is_enabled)
     .WithCallback(overworld::PokemonFlight::Apply)
     .WithDescription("The player sits on a flying Pokemon: the Circle Pad "
                      "gives the direction, R goes up and L goes down. No "
                      "wild Pokemon, no collision. The Pokemon of the team "
                      "that follows you leaves during the flight.")
     .AddSpecies("Flying Pokemon",
                 overworld::PokemonFlight::GetInstance().species)
     .WithBounds(1, static_cast<u32>(SpeciesId::kCount) - 1)
     .Add("Flight Pokemon Size", overworld::PokemonFlight::GetInstance().scale)
     .WithFactor(0.1f)
     .Add("Flight Speed", overworld::PokemonFlight::GetInstance().speed)
     .WithFactor(0.25f)
     .Add("Climb Speed", overworld::PokemonFlight::GetInstance().climb_speed)
     .WithFactor(0.5f)
     .Add("Ride A Pokemon", overworld::PokemonFlight::GetInstance().is_ride_enabled)
     .WithCallback(overworld::PokemonFlight::Apply)
     .WithDescription("The player sits on a Pokemon with four legs and rides "
                      "it on the ground: walk as usual, the Pokemon walks "
                      "with you. The flight has the priority when both are "
                      "on. The Pokemon of the team that follows you leaves.")
     .AddSpecies("Riding Pokemon",
                 overworld::PokemonFlight::GetInstance().ride_species)
     .WithBounds(1, static_cast<u32>(SpeciesId::kCount) - 1)
     .Add("Ride Pokemon Size", overworld::PokemonFlight::GetInstance().ride_scale)
     .WithFactor(0.05f)
     .Add("Ride Speed", overworld::PokemonFlight::GetInstance().ride_speed)
     .WithFactor(0.25f)
     .WithDescription("How fast the player goes on the mount: 1 is the speed "
                      "of the game, 2 is two times faster.")
     .Add("Ride Height", overworld::PokemonFlight::GetInstance().ride_height)
     .WithFactor(2.0f)
     .WithDescription("The height of the seat on the back of the Pokemon.")
     .Add("Ride Leg Spread", overworld::PokemonFlight::GetInstance().ride_spread)
     .WithFactor(0.1f)
     .WithDescription("How far the legs of the player go to the sides.")
     .Add("Ride Back", overworld::PokemonFlight::GetInstance().ride_back)
     .WithFactor(2.0f)
     .WithDescription("The seat goes back along the Pokemon: more is closer "
                      "to the tail.")
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
