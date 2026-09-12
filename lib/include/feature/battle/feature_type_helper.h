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

#pragma once

#include "common.h"
#include "game/battle/manager.h"
#include "game/core/process_manager.h"
#include "game/global_data/move.h"
#include "ui/main_application.h"

namespace feature {
class TypeHelper {
  MAKE_SINGLETON(TypeHelper)
  struct EffectivenessInfo {
    const c16* symbol;
    Color color;
  };

  STATIC_INLINE EffectivenessInfo GetEffectivenessInfo(u8 result) {
    static const c16* symbols[] = {
        u"[0]", u"[1/64]", u"[1/32]", u"[1/16]", u"[1/8]", u"[1/4]", u"[1/2]",
        u"[1]", u"[x2]", u"[x4]", u"[x8]", u"[x16]", u"[x32]", u"[x64]"
    };

    if (result >= 14) {
      return {u"[?]", Color{1.0f, 1.0f, 1.0f, 1.0f}};
    }

    Color color;
    if (result == 0) {
      color = Color{0.5f, 0.5f, 0.5f, 1.0f}; // Gris
    } else if (result < 7) {
      color = Color{1.0f, 0.2f, 0.2f, 1.0f}; // Rouge
    } else if (result == 7) {
      color = Color{1.0f, 1.0f, 1.0f, 1.0f}; // Blanc
    } else {
      color = Color{0.2f, 1.0f, 0.2f, 1.0f}; // Vert
    }

    return {symbols[result], color};
  }

public:
  static void DrawTop() {
    if (ui::MainApplication::GetInstance().IsOpened()) return;

    const bool is_battle = game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_BATTLE_VTABLE);
    if (!is_battle) return;

    auto* attacker = battle::Manager::GetPokemon(true, 0, 0);
    auto* defender = battle::Manager::GetPokemon(true, 1, 0);
    if (attacker == nullptr || defender == nullptr) return;

    Graphics::EnableScissor(0, 0, 120, 90);
    Graphics::FillScreen(0, 0, 0, 0.4f);
    Graphics::SetTextScale(0.5, 0.5);

    s32 x = 2;
    s32 y = 2;
    const s32 y_offset = 14;

    c16 title_buffer[BUFFER_SIZE];
    Utils::Format(title_buffer, u"<Type Helper>");
    Graphics::DrawText(x, y, title_buffer, Color{1.0f, 0.8f, 0.2f, 1.0f});
    y += y_offset*2;

    const u16 def_types = ((u16 (*)(const battle::Pokemon*))
        ADDRESS_BATTLE_GET_POKEMON_TYPE_PAIR)(defender);

    for (int i = 0; i < 4; ++i) {
      MoveId move_id = attacker->moves[i].core.id;
      if (move_id == MoveId::kNone) continue;

      GetMoveString(static_cast<u16>(move_id));
      auto& move_data = global_data::Move::GetInstance(move_id);

      u8 result = ((u8 (*)(MoveType, u16))ADDRESS_BATTLE_TYPE_AFFINITY_PAIR)(
          move_data.type, def_types);

      auto info = GetEffectivenessInfo(result);

      c16 buffer[BUFFER_SIZE];
      Utils::Format(buffer, u"%ls %ls", info.symbol, String::GetTmpBuf());
      Graphics::DrawText(x, y, buffer, info.color);

      y += y_offset;
    }

    Graphics::DisableScissor();
  }

  STATIC_INLINE void GetMoveString(u16 move) {
    ((void (*)(u16, String*))ADDRESS_GET_MOVE_NAME)(move, String::GetTmpStr());
  }
};
} // namespace feature