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

#include "battle/patch/type_helper.h"
#include "battle/patch/battle.h"
#include "battle/native/manager.h"
#include "core/native/process_manager.h"
#include "pokemon/native/move_data.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"

namespace battle {

TypeHelper::EffectivenessInfo TypeHelper::GetEffectivenessInfo(u8 result) {
  static const c16* symbols[] = {
      u"[0]", u"[1/64]", u"[1/32]", u"[1/16]", u"[1/8]", u"[1/4]", u"[1/2]",
      u"[1]", u"[x2]", u"[x4]", u"[x8]", u"[x16]", u"[x32]", u"[x64]"
  };

  if (result >= 14) {
    return {u"[?]", Color{1.0f, 1.0f, 1.0f, 1.0f}};
  }

  Color color;
  if (result == 0) {
    color = Color{0.5f, 0.5f, 0.5f, 1.0f};
  } else if (result < 7) {
    color = Color{1.0f, 0.2f, 0.2f, 1.0f};
  } else if (result == 7) {
    color = Color{1.0f, 1.0f, 1.0f, 1.0f};
  } else {
    color = Color{0.2f, 1.0f, 0.2f, 1.0f};
  }

  return {symbols[result], color};
}

void TypeHelper::GetMoveString(MoveId move) {
  ((void (*)(MoveId, String*))pokemon::address::kGetMoveName)(
      move, String::GetTmpStr());
}

void TypeHelper::DrawTop() {
  if (!Battle::GetInstance().show_type_helper) return;
  if (ui::MainApplication::GetInstance().IsOpened()) return;

  const bool is_battle = core::ProcessManager::GetInstance().IsCurrentProcess(
      address::kVtable);
  if (!is_battle) return;

  auto* attacker = Manager::GetPokemon(true, 0, 0);
  auto* defender = Manager::GetPokemon(true, 1, 0);
  if (attacker == nullptr || defender == nullptr) return;

  sys::Graphics::EnableScissor(0, 0, 120, 90);
  sys::Graphics::FillScreen(0, 0, 0, 0.4f);
  sys::Graphics::SetTextScale(0.5, 0.5);

  s32 x = 2;
  s32 y = 2;
  const s32 y_offset = 14;

  c16 title_buffer[sys::address::kBufferSize];
  core::Utils::Format(title_buffer, u"<Type Helper>");
  sys::Graphics::DrawText(x, y, title_buffer, Color{1.0f, 0.8f, 0.2f, 1.0f});
  y += y_offset * 2;

  const u16 def_types = ((u16 (*)(const Pokemon*))
      address::kGetPokemonTypes)(defender);

  for (const auto & move : attacker->moves) {
    MoveId move_id = move.core.id;
    if (move_id == MoveId::kNone) continue;

    GetMoveString(move_id);
    auto& move_data = pokemon::MoveData::GetInstance(move_id);

    u8 result = ((u8 (*)(TypeId, u16))address::kGetEffectiveness)(
        move_data.type, def_types);

    auto info = GetEffectivenessInfo(result);

    c16 buffer[sys::address::kBufferSize];
    core::Utils::Format(buffer, u"%ls %ls", info.symbol, String::GetTmpBuf());
    sys::Graphics::DrawText(x, y, buffer, info.color);

    y += y_offset;
  }

  sys::Graphics::DisableScissor();
}

} // namespace battle
