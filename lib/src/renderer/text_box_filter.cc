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

#include "renderer/patch/text_box_filter.h"

#include "core/hook_manager.h"
#include "renderer/native/text_box.h"

namespace renderer {

void TextBoxFilter::Initialize() {
  core::HookManager::Initialize(HookId::kDrawTextBox,
                          address::kDrawTextBox, (uptr)DrawHook);
}

u32 TextBoxFilter::DrawHook(TextBox* text_box, u32 writer) {
  u32 res = core::HookManager::Call<u32>(HookId::kDrawTextBox, text_box, writer);

  auto& ctx = GetInstance();
  if (ctx.is_enabled) {
    text_box->pane.scale = ctx.scale;
    text_box->top_color = ctx.top_color;
    text_box->bottom_color = ctx.bottom_color;
  }

  return res;
}

} // namespace renderer
