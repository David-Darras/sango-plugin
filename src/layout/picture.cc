#include "layout/picture.h"

#include <cmath>

#include "hack/hook_manager.h"

namespace layout {

static float time = 0.0f;

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
  constexpr float kSpeed = 0.01f;
  time += kSpeed;

  float pulse = 1.0f + sinf(time * 2.0f) * 0.05f;
  picture->pane.scale.x = pulse;
  picture->pane.scale.y = pulse;

  float baseHue = fmodf(time * 0.1f, 1.0f);

  Color8 c0 = HSVtoRGB(fmodf(baseHue + 0.00f, 1.0f), 1.0f, 1.0f);  // top-left
  Color8 c1 = HSVtoRGB(fmodf(baseHue + 0.25f, 1.0f), 1.0f, 1.0f);  // top-right
  Color8 c2 =
      HSVtoRGB(fmodf(baseHue + 0.50f, 1.0f), 1.0f, 1.0f);  // bottom-right
  Color8 c3 =
      HSVtoRGB(fmodf(baseHue + 0.75f, 1.0f), 1.0f, 1.0f);  // bottom-left

  picture->colors[0] = c0;
  picture->colors[1] = c1;
  picture->colors[2] = c2;
  picture->colors[3] = c3;

  HookManager::GetInstance()
      .Get(HookID::kOnDrawPicture)
      ->CallOriginal<u32>(picture, p0, p1, p2);
}

}  // namespace layout