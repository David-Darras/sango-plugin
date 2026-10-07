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
 * The overlay: every page of the library under one menu, for exploring and
 * editing the game while it runs.
 */

#include "plugin.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/move_animation.h"
#include "core/native/process_manager.h"
#include "net/network.h"
#include "net/remote_avatars.h"
#include "savedata/native/pokemon_team.h"
#include "overworld/native/model_manager.h"
#include "ui/page/pages.h"
#include "ui/painter.h"

namespace script {
void Install();
}

namespace battle {
void RegisterAbsoluteZeroAnimation();
void RegisterThunderboltAnimation();
void RegisterThunderboltLayersAnimation();
void RegisterEffectShowcaseAnimation();
}

static void GiveTestMovesToFirstPokemon() {
  static bool done = false;
  if (done || !core::ProcessManager::IsOverworldActive()) return;

  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0 || team.pokemons[0] == nullptr) return;

  auto& pokemon = *team.pokemons[0];
  pokemon.accessor->Decrypt();
  pokemon.core->moves[0] = kMoveAbsoluteZero;
  pokemon.core->pp[0] = 50;
  pokemon.core->pp_up_count[0] = 0;
  pokemon.core->moves[1] = MoveId::kThunderbolt;
  pokemon.core->pp[1] = 24;
  pokemon.core->pp_up_count[1] = 3;
  pokemon.accessor->Encrypt();
  done = true;
}

static void EveryFrame() {
  GiveTestMovesToFirstPokemon();
  net::Network::Update();
  plugin::UpdateFrame();
#ifdef GAME_ORAS
  net::RemoteAvatars::Update();
#endif
  // UpdateFollowingPokemon();
  plugin::DrawFrame();
}

void Initialize() {
  plugin::InitializeEngine();

  script::Install();
#ifdef GAME_ORAS
  net::RemoteAvatars::Initialize();
#endif

  // battle::RegisterThunderboltLayersAnimation();

  // plugin::LoadConfiguration();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), ui::LoadTopPage);
  plugin::Start(EveryFrame);
}