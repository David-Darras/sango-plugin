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

#include "layout/text_box.h"
#include "menu/plugin_menu.h"

#include "hack/hook_manager.h"

namespace layout {
static struct {
  bool is_enabled;
  Vec2 scale;
  Color8 top_color;
  Color8 bottom_color;
} ctx = {
    .is_enabled = false,
    .scale = {1.0f, 1.0f},
    .top_color = {255, 255, 255, 255},
    .bottom_color = {255, 255, 255, 255}
};

void OnDrawTextBox(TextBox* text_box, u32 writer) {
  HookManager::GetInstance()
      .Get(HookID::kOnDrawTextBox)
      ->CallOriginal<u32>(text_box, writer);
  if (ctx.is_enabled) {
    text_box->pane.scale = ctx.scale;
    text_box->top_color = ctx.top_color;
    text_box->bottom_color = ctx.bottom_color;
  }
}

void TextBox::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager::GetInstance().Add(HookID::kOnDrawTextBox,
                                 ADDRESS_LAYOUT_DRAW_TEXT_BOX,
                                 (uptr)layout::OnDrawTextBox);

  menu.Add("Is Enabled", ctx.is_enabled)
      .Add("Scale X", ctx.scale.x)
      .WithFactor(0.1f)
      .Add("Scale Y", ctx.scale.y)
      .WithFactor(0.1f)
      .AddSeparator()
      .Add("Top Color - Red", ctx.top_color.r)
      .Add("Top Color - Green", ctx.top_color.g)
      .Add("Top Color - Blue", ctx.top_color.b)
      .Add("Top Color - Alpha", ctx.top_color.a)
      .AddSeparator()
      .Add("Bottom Color - Red", ctx.bottom_color.r)
      .Add("Bottom Color - Green", ctx.bottom_color.g)
      .Add("Bottom Color - Blue", ctx.bottom_color.b)
      .Add("Bottom Color - Alpha", ctx.bottom_color.a);
}
} // namespace layout