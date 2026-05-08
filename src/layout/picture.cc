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

#include "layout/picture.h"

#include <cmath>

#include "hack/hook_manager.h"
#include "menu/plugin_menu.h"

namespace layout {
static struct {
  bool psychedelic_vision;
  bool is_enabled;
  u8 alpha;
  f32 time;
  Vec2 scale;
  Color8 colors[4];
} ctx = {
    .psychedelic_vision = false,
    .is_enabled = false,
    .alpha = 255,
    .time = 0.0f,
    .scale = {1.0f, 1.0f},
    .colors = {
        {255, 255, 255, 255},
        {255, 255, 255, 255},
        {255, 255, 255, 255},
        {255, 255, 255, 255},
    },
};

static Color8 HSVtoRGB(float h, float s, float v) {
  float c = v * s;
  float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
  float m = v - c;

  float r = 0, g = 0, b = 0;

  if (h < 1.0f / 6.0f) {
    r = c;
    g = x;
    b = 0;
  } else if (h < 2.0f / 6.0f) {
    r = x;
    g = c;
    b = 0;
  } else if (h < 3.0f / 6.0f) {
    r = 0;
    g = c;
    b = x;
  } else if (h < 4.0f / 6.0f) {
    r = 0;
    g = x;
    b = c;
  } else if (h < 5.0f / 6.0f) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  return Color8{(u8)((r + m) * 255), (u8)((g + m) * 255), (u8)((b + m) * 255),
                255};
}

static Color8 LerpColor(Color8 a, Color8 b, float t) {
  return Color8{(u8)(a.r + (b.r - a.r) * t), (u8)(a.g + (b.g - a.g) * t),
                (u8)(a.b + (b.b - a.b) * t), (u8)(a.a + (b.a - a.a) * t)};
}

void OnDrawPicture(Picture* picture, u32 p0, u32 p1, u32 p2) {
  if (ctx.psychedelic_vision) {
    constexpr float kSpeed = 0.01f;
    ctx.time += kSpeed;

    float pulse = 1.0f + sinf(ctx.time * 2.0f) * 0.05f;
    picture->pane.scale.x = pulse;
    picture->pane.scale.y = pulse;

    float baseHue = fmodf(ctx.time * 0.1f, 1.0f);

    Color8 c0 = HSVtoRGB(fmodf(baseHue + 0.00f, 1.0f), 1.0f, 1.0f); // top-left
    Color8 c1 = HSVtoRGB(fmodf(baseHue + 0.25f, 1.0f), 1.0f, 1.0f); // top-right
    Color8 c2 =
        HSVtoRGB(fmodf(baseHue + 0.50f, 1.0f), 1.0f, 1.0f); // bottom-right
    Color8 c3 =
        HSVtoRGB(fmodf(baseHue + 0.75f, 1.0f), 1.0f, 1.0f); // bottom-left

    picture->colors[0] = c0;
    picture->colors[1] = c1;
    picture->colors[2] = c2;
    picture->colors[3] = c3;
  }
  if (ctx.is_enabled) {
    picture->pane.scale = ctx.scale;
    picture->pane.alpha = ctx.alpha;
    for (u32 i = 0; i < 4; i++) {
      picture->colors[i].r = ctx.colors[i].r;
      picture->colors[i].g = ctx.colors[i].g;
      picture->colors[i].b = ctx.colors[i].b;
    }
  }

  HookManager::GetInstance()
      .Get(HookID::kOnDrawPicture)
      ->CallOriginal<u32>(picture, p0, p1, p2);
}

void Picture::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager::GetInstance().Add(HookID::kOnDrawPicture,
                                 ADDRESS_LAYOUT_DRAW_PICTURE,
                                 (uptr)layout::OnDrawPicture);

  menu.Add("Psychedelic Vision", ctx.psychedelic_vision)
      .AddSeparator()
      .Add("Is Enabled", ctx.is_enabled)
      .Add("Scale X", ctx.scale.x)
      .WithFactor(0.1f)
      .Add("Scale Y", ctx.scale.y)
      .WithFactor(0.1f)
      .Add("Alpha", ctx.alpha)

      .AddSeparator()
      .Add("Top Left Corner - Red", ctx.colors[0].r)
      .Add("Top Left Corner - Green", ctx.colors[0].g)
      .Add("Top Left Corner - Blue", ctx.colors[0].b)

      .AddSeparator()
      .Add("Top Right Corner - Red", ctx.colors[1].r)
      .Add("Top Right Corner - Green", ctx.colors[1].g)
      .Add("Top Right Corner - Blue", ctx.colors[1].b)

      .AddSeparator()
      .Add("Bottom Left Corner - Red", ctx.colors[2].r)
      .Add("Bottom Left Corner - Green", ctx.colors[2].g)
      .Add("Bottom Left Corner - Blue", ctx.colors[2].b)

      .AddSeparator()
      .Add("Bottom Right Corner - Red", ctx.colors[3].r)
      .Add("Bottom Right Corner - Green", ctx.colors[3].g)
      .Add("Bottom Right Corner - Blue", ctx.colors[3].b);
}
} // namespace layout