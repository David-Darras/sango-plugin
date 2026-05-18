#include "core/pokemon_model.h"

#include <cmath>

#include "hack/hook_manager.h"
#include "menu/log_menu.h"

static FORCE_INLINE void SaturatePixel(u8& r, u8& g, u8& b) {
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

static FORCE_INLINE void GetTextureInfo(TextureFormat& format, u16& width,
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

static FORCE_INLINE void ApplyFilterToPokemon(Resource& resource) {
  if (!resource.IsValid()) return;

  const u32 count = resource.GetContent().textures.count;

  for (u32 i = 0; i < count; ++i) {
    u32 cmd_count;
    u32* cmd = resource.GetGpuCommands(i, cmd_count);

    TextureFormat format;
    u16 width, height;
    u32 address;
    GetTextureInfo(format, width, height, address, cmd, cmd_count);

    u8* pixels = (u8*)address;

    if (format == rgba8) {
      u8* end = pixels + width * height * 4;
      while (pixels < end) {
        SaturatePixel(pixels[3], pixels[2], pixels[1]);
        pixels += 4;
      }
    } else if (format == rgb8) {
      u8* end = pixels + width * height * 3;
      while (pixels < end) {
        SaturatePixel(pixels[2], pixels[1], pixels[0]);
        pixels += 3;
      }
    }
  }
}

void OnCreatePokemonModel(PokemonModel* model, u32 a, u32 b) {
  HookManager::GetInstance().Get(HookID::kOnCreatePokemonModel)->CallOriginal<
    void>(model, a, b);
  ApplyFilterToPokemon(model->GetTexture());
}