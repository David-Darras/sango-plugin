#include "core/pokemon_model.h"

#include <cmath>

#include "hack/hook_manager.h"
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"

static struct {
  u8 filter;
  u8 red;
  u8 green;
  u8 blue;
} ctx;

STATIC_INLINE void PermuteBGR(u8& r, u8& g, u8& b) {
  u8 tmp = r;
  r = b;
  b = tmp;
}

STATIC_INLINE void PermuteGRB(u8& r, u8& g, u8& b) {
  u8 tmp = r;
  r = g;
  g = tmp;
}

STATIC_INLINE void PermuteRBG(u8& r, u8& g, u8& b) {
  u8 tmp = g;
  g = b;
  b = tmp;
}

STATIC_INLINE void PermuteBRG(u8& r, u8& g, u8& b) {
  u8 old_r = r;
  u8 old_g = g;
  r = b;
  g = old_r;
  b = old_g;
}

STATIC_INLINE void PermuteGBR(u8& r, u8& g, u8& b) {
  u8 old_r = r;
  u8 old_g = g;
  r = old_g;
  g = b;
  b = old_r;
}

STATIC_INLINE void GrayscalePixel(u8& r, u8& g, u8& b) {
  const u8 gray = (u8)((77 * r + 150 * g + 29 * b) >> 8);
  r = g = b = gray;
}

STATIC_INLINE void InvertPixel(u8& r, u8& g, u8& b) {
  r ^= 0xFF;
  g ^= 0xFF;
  b ^= 0xFF;
}

STATIC_INLINE void WarmPixel(u8& r, u8& g, u8& b) {
  r = r + 20 > 255 ? 255 : r + 20;
  b = b < 20 ? 0 : b - 20;
}

STATIC_INLINE void CoolPixel(u8& r, u8& g, u8& b) {
  b = b + 20 > 255 ? 255 : b + 20;
  r = r < 20 ? 0 : r - 20;
}

STATIC_INLINE void ThresholdPixel(u8& r, u8& g, u8& b) {
  const u8 luma = (u8)((77 * r + 150 * g + 29 * b) >> 8);
  const u8 v = luma >= 128 ? 255 : 0;
  r = g = b = v;
}

STATIC_INLINE void SepiaPixel(u8& r, u8& g, u8& b) {
  const u16 sr = (u16)((r * 112 + g * 88 + b * 56) >> 8);
  const u16 sg = (u16)((r * 100 + g * 78 + b * 48) >> 8);
  const u16 sb = (u16)((r * 78 + g * 61 + b * 38) >> 8);
  r = sr > 255 ? 255 : (u8)sr;
  g = sg > 255 ? 255 : (u8)sg;
  b = sb > 255 ? 255 : (u8)sb;
}

STATIC_INLINE void SaturatePixel(u8& r, u8& g, u8& b) {
  const s16 gray = (77 * r + 150 * g + 29 * b) >> 8;

  const s16 dr = (s16)r - gray;
  const s16 dg = (s16)g - gray;
  const s16 db = (s16)b - gray;

  s16 nr = gray + ((dr * 640) >> 8);
  s16 ng = gray + ((dg * 640) >> 8);
  s16 nb = gray + ((db * 640) >> 8);

  const s16 max_val = (gray > 128) ? gray : 128;

  nr = (nr * (384 - max_val)) >> 8;
  ng = (ng * (384 - max_val)) >> 8;
  nb = (nb * (384 - max_val)) >> 8;

  r = (u8)(nr < 0 ? 0 : nr > 240 ? 240 : nr);
  g = (u8)(ng < 0 ? 0 : ng > 240 ? 240 : ng);
  b = (u8)(nb < 0 ? 0 : nb > 240 ? 240 : nb);
}

STATIC_INLINE void FillPixel(u8& r, u8& g, u8& b) {
  r = ctx.red;
  g = ctx.green;
  b = ctx.blue;
}

STATIC_INLINE void GetTextureInfo(TextureFormat& format, u16& width,
                                        u16& height,
                                        u32& address, const u32* cmd,
                                        u32 count) {
  u32 idx = 0;

  while (idx < count) {
    const u32 param = cmd[idx];
    const u32 header = cmd[idx + 1];
    const u16 id = (u16)(header & 0xFFFF);

    if (id == 0x008E) {
      format = (TextureFormat)param;
    } else if (id == 0x0082) {
      width = (param >> 16) & 0xFFFF;
      height = param & 0xFFFF;
    } else if (id == 0x0085) {
      address = (param << 3) - 0x0C000000;
    }

    idx += 2;
  }
}

#define APPLY_FILTER(filter_name, filter_func) case filter_name: {\
  if (format == rgba8) {\
    u8* end = pixels + width * height * 4;\
    while (pixels < end) {\
      filter_func(pixels[3], pixels[2], pixels[1]);\
      pixels += 4;\
    }\
  } else if (format == rgb8) {\
    u8* end = pixels + width * height * 3;\
    while (pixels < end) {\
      filter_func(pixels[2], pixels[1], pixels[0]);\
      pixels += 3;\
    }\
  }\
  break;\
}

STATIC_INLINE void ApplyFilterToPokemon(Resource& resource) {
  if (!resource.IsValid() || ctx.filter == 0) return;

  const u32 count = resource.GetContent().textures.count;

  for (u32 i = 0; i < count; ++i) {
    u32 cmd_count;
    u32* cmd = resource.GetGpuCommands(i, cmd_count);

    TextureFormat format;
    u16 width, height;
    u32 address;
    GetTextureInfo(format, width, height, address, cmd, cmd_count);

    u8* pixels = (u8*)address;

    switch (ctx.filter) {
      APPLY_FILTER(1, PermuteBGR)
      APPLY_FILTER(2, PermuteGRB)
      APPLY_FILTER(3, PermuteRBG)
      APPLY_FILTER(4, PermuteBRG)
      APPLY_FILTER(5, PermuteGBR)
      APPLY_FILTER(6, SaturatePixel)
      APPLY_FILTER(7, GrayscalePixel)
      APPLY_FILTER(8, InvertPixel)
      APPLY_FILTER(9, WarmPixel)
      APPLY_FILTER(10, CoolPixel)
      APPLY_FILTER(11, ThresholdPixel)
      APPLY_FILTER(12, SepiaPixel)
      APPLY_FILTER(13, FillPixel)
    }
  }
}

void OnCreatePokemonModel(PokemonModel* model, u32 a, u32 b) {
  HookManager::GetInstance().Get(HookID::kOnCreatePokemonModel)->CallOriginal<
    void>(model, a, b);
  ApplyFilterToPokemon(model->GetTexture());
}

void PokemonModelMenu(menu::PluginMenu& menu, void* args) {
  static const char* FILTERS[] = {
      "RGB",
      "BGR",
      "GRB",
      "RBG",
      "BRG",
      "GBR",
      "Saturate",
      "Grayscale",
      "Invert",
      "Warm",
      "Cool",
      "Threshold",
      "Sepia",
      "Fill"
  };

  menu.Add("Filter", ctx.filter)
      .WithArray(FILTERS, SIZE(FILTERS))
      .Add("Fill - Red", ctx.red)
      .Add("Fill - Green", ctx.green)
      .Add("Fill - Blue", ctx.blue);
}