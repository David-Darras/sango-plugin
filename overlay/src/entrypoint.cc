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
 * @file entrypoint.cc
 * @brief The overlay: all the pages of the library in one menu.
 *
 * Use the overlay to explore and to change the game while it runs.
 */

#include "plugin.h"
#include "custom_battle.h"
#include "net/network.h"
#include "net/remote_avatars.h"
#include "ui/page/pages.h"
#include "ui/painter.h"

namespace script {
void Install();
}

namespace battle {
void RegisterAbsoluteZeroAnimation();
void RegisterThunderboltAnimation();
}

static void EveryFrame() {
  net::Network::Update();
  plugin::UpdateFrame();
#ifdef GAME_ORAS
  net::RemoteAvatars::Update();
#endif
  plugin::DrawFrame();
}

void Initialize() {
  plugin::InitializeEngine();
  battle::RegisterCustomAbilities();
  battle::RegisterCustomMoves();
  battle::RegisterAbsoluteZeroAnimation();
  battle::RegisterThunderboltAnimation();

  script::Install();
#ifdef GAME_ORAS
  net::RemoteAvatars::Initialize();
#endif

  plugin::OpenMenu(ui::MainAppPainter::GetInstance(), ui::LoadTopPage);
  plugin::Start(EveryFrame);
}
