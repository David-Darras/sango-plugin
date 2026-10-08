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
 * @file effect_style.h
 * @brief Changes the particle effects of the battles: colors, size, life...
 *
 * Use the functions of battle::MoveAnimation (ColorEffect()...) instead.
 */

#pragma once

#include "common.h"
#include "core/color.h"
#include "core/native/archive_input.h"

namespace battle {

/// Changes the particle files of the battles when the game loads them.
class EffectStyles {
  MAKE_SINGLETON(EffectStyles)

public:
  static constexpr u32 kMaxEntries = 64;
  static constexpr u32 kMaxTrackedReads = 64;
  static constexpr u32 kMaxLayers = 8;
  static constexpr u32 kAllLayers = 0xFFFFFFFF;

  static bool Tint(u32 effect_file, const Color8& multiplier,
                   u32 layer = kAllLayers);
  static bool Recolor(u32 effect_file, const Color8& start,
                      const Color8& middle, const Color8& end,
                      u32 layer = kAllLayers);
  static bool ScaleLife(u32 effect_file, f32 factor, u32 layer = kAllLayers);
  static bool ScaleDensity(u32 effect_file, f32 factor,
                           u32 layer = kAllLayers);
  static bool ScaleSize(u32 effect_file, f32 factor, u32 layer = kAllLayers);
  static bool Fade(u32 effect_file, f32 fade_in_end, f32 fade_out_start,
                   f32 peak, u32 layer = kAllLayers);
  static bool Loop(u32 effect_file, u32 layer = kAllLayers);
  static bool Disable(u32 effect_file, u32 layer = kAllLayers);

  static void Remove(u32 effect_file);
  static void Clear();
  static bool Has(u32 effect_file);

  static void TrackRead(const core::ArchiveInput* input);
  static void Apply(uptr resource_buffer);

private:
  enum class ColorMode : u8 {
    kNone,
    kTint,
    kRecolor,
  };

  struct Entry {
    u32 effect_file;
    u32 layer;
    ColorMode color_mode;
    Vec3 color_first;
    Vec3 color_second;
    Vec3 color_third;
    bool has_life;
    f32 life_scale;
    bool has_density;
    f32 density_scale;
    bool has_size;
    f32 size_scale;
    bool has_fade;
    f32 fade_in_end;
    f32 fade_out_start;
    f32 fade_peak;
    bool has_loop;
    bool has_disable;
  };

  struct ResolvedEntries {
    Entry all;
    bool has_all;
    Entry layers[kMaxLayers];
    bool has_layer[kMaxLayers];
  };

  struct TrackedRead {
    uptr destination;
    u32 effect_file;
  };

  static Entry* Acquire(u32 effect_file, u32 layer);
  static void Reset(Entry* entry, u32 effect_file, u32 layer);
  static bool Resolve(u32 effect_file, ResolvedEntries* resolved);
  static Entry Combine(const ResolvedEntries& resolved, u32 layer);
  static bool SameString(uptr first, uptr second, uptr end);
  static bool FindFileOf(uptr resource_buffer, u32* effect_file);
  static void PatchBuffer(uptr buffer, const ResolvedEntries& resolved);
  static void PatchEmitter(uptr emitter, const Entry& entry);
  static void PatchSet(uptr buffer, uptr set, const Entry& entry);
  static void PatchSetCollection(uptr set, uptr collection, const Entry& entry);
  static void PatchInitializer(uptr initializer, const Entry& entry,
                               u32 size_target);
  static void PatchUpdater(uptr updater, const Entry& entry, u32 size_target);
  static void PatchColorUpdater(uptr updater, const Entry& entry);
  static void PatchFadeUpdater(uptr updater, const Entry& entry);
  static void PatchSizeUpdater(uptr updater, const Entry& entry);

  Entry entries_[kMaxEntries];
  u32 entry_count_ = 0;
  TrackedRead reads_[kMaxTrackedReads];
  u32 read_count_ = 0;
  u32 next_read_ = 0;
};

}
