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
 * @file effect_style.cc
 * @brief Changes the particle effects of the battles: colors, size, life...
 *
 * The declarations are in battle/patch/effect_style.h.
 */

#include "battle/patch/effect_style.h"
#include "core/constant/archive_id.h"

namespace battle {

namespace {

constexpr bool kSupported = GAME_CONSTANT(false, true);

constexpr u32 kGraphicsFileMagic = 0x58464743;
constexpr u32 kSetSignature = 0x444F4E43;
constexpr u32 kEmitterSignature = 0x544D4550;
constexpr u32 kMaxGraphicsFileSize = 0x100000;
constexpr u32 kFileSizeOffset = 12;
constexpr u32 kSignatureOffset = 4;
constexpr u32 kSetMinimumSize = 0x60;

constexpr u32 kSetCollectionOffset = 0x30;
constexpr u32 kSetInitializerCountOffset = 0x34;
constexpr u32 kSetInitializerTableOffset = 0x38;
constexpr u32 kSetUpdaterCountOffset = 0x3C;
constexpr u32 kSetUpdaterTableOffset = 0x40;
constexpr u32 kSetPatchedOffset = 0x53;
constexpr u8 kSetLifePatched = 0x01;
constexpr u8 kSetCapacityPatched = 0x02;
constexpr u32 kMaxSetEntries = 32;

constexpr u32 kCollectionCapacityOffset = 0;
constexpr u32 kCollectionAttributeCountOffset = 4;
constexpr u32 kCollectionAttributeTableOffset = 8;
constexpr u32 kMaxAttributes = 32;
constexpr u32 kAttributeTypeOffset = 0;
constexpr u32 kAttributeUsageOffset = 4;
constexpr u32 kAttributeDimensionOffset = 8;
constexpr u32 kAttributeCountOffset = 12;
constexpr u32 kAttributeTableOffset = 16;
constexpr u32 kParameterAttributeType = 0x40000000;
constexpr u32 kMaxParameterValues = 16;

constexpr u32 kEmitterMinimumSize = 0xE0;
constexpr u32 kMaxEmitters = 64;
constexpr u32 kNameOffset = 0x0C;
constexpr u32 kEmitterSetPathOffset = 0xB8;
constexpr u32 kEmitterPatchedOffset = 0xC1;
constexpr u32 kEmitterSpanInfiniteOffset = 0xD4;
constexpr u32 kEmitterRatioOffset = 0xC4;

constexpr u32 kObjectTypeOffset = 0;
constexpr u32 kObjectEnabledOffset = 5;
constexpr u32 kObjectPatchedOffset = 6;
constexpr u32 kObjectTargetOffset = 8;
constexpr u8 kPatchedMarker = 0xC5;

constexpr u32 kScaleTarget = 1;
constexpr u32 kColorTarget = 3;
constexpr u32 kAlphaTarget = 4;
constexpr u32 kScaleExtendedTarget = 8;
constexpr u32 kLifeTarget = 11;

constexpr u32 kColorInitializerType = 0x04000000;
constexpr u32 kColorInitializerValue = 12;
constexpr u32 kScaleInitializerType = 0x00200000;
constexpr u32 kScaleInitializerBase = 12;
constexpr u32 kLifeInitializerType = 0x01000000;
constexpr u32 kLifeInitializerMaximum = 12;
constexpr u32 kLifeInitializerMinimum = 16;

constexpr u32 kFixedVectorUpdaterType = 0x00800001;
constexpr u32 kFixedVectorUpdaterValue = 16;
constexpr u32 kKeyedVectorUpdaterType = 0x00100000;
constexpr u32 kKeyedNumberUpdaterType = 0x00200000;
constexpr u32 kFixedNumberUpdaterType = 0x00400001;
constexpr u32 kFixedNumberUpdaterValue = 16;

constexpr u32 kKeyedInTime = 16;
constexpr u32 kKeyedOutTime = 20;
constexpr u32 kKeyedStart = 24;
constexpr u32 kKeyedMiddle = 36;
constexpr u32 kKeyedInSlope = 48;
constexpr u32 kKeyedOutSlope = 60;
constexpr u32 kKeyedOutOffset = 72;
constexpr u32 kKeyedVectorSize = 84;
constexpr u32 kNumberKeyedSize = 44;
constexpr f32 kTimeScale = 65536.0f;

Vec3 ToUnit(const Color8& color) {
  return Vec3(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
}

Vec3 ReadVec3(uptr address) {
  return Vec3(READF(address), READF(address + 4), READF(address + 8));
}

void WriteVec3(uptr address, const Vec3& value) {
  WRITEF(address, value.x);
  WRITEF(address + 4, value.y);
  WRITEF(address + 8, value.z);
}

Vec3 Multiply(const Vec3& a, const Vec3& b) {
  return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

Vec3 Subtract(const Vec3& a, const Vec3& b) {
  return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec3 Scale(const Vec3& a, f32 factor) {
  return Vec3(a.x * factor, a.y * factor, a.z * factor);
}

void ScaleVec3InPlace(uptr address, f32 factor) {
  WriteVec3(address, Scale(ReadVec3(address), factor));
}

s32 ToFixed(f32 value) {
  return (s32)(value * kTimeScale + (value >= 0.0f ? 0.5f : -0.5f));
}

f32 Clamp(f32 value, f32 low, f32 high) {
  return value < low ? low : (value > high ? high : value);
}

}

void EffectStyles::Reset(Entry* entry, u32 effect_file, u32 layer) {
  entry->effect_file = effect_file;
  entry->layer = layer;
  entry->color_mode = ColorMode::kNone;
  entry->color_first = Vec3();
  entry->color_second = Vec3();
  entry->color_third = Vec3();
  entry->has_life = false;
  entry->life_scale = 1.0f;
  entry->has_density = false;
  entry->density_scale = 1.0f;
  entry->has_size = false;
  entry->size_scale = 1.0f;
  entry->has_fade = false;
  entry->fade_in_end = 0.0f;
  entry->fade_out_start = 1.0f;
  entry->fade_peak = 1.0f;
  entry->has_loop = false;
  entry->has_disable = false;
}

EffectStyles::Entry* EffectStyles::Acquire(u32 effect_file, u32 layer) {
  if (!kSupported) return nullptr;
  if (layer != kAllLayers && layer >= kMaxLayers) return nullptr;
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.entry_count_; i++) {
    if (ctx.entries_[i].effect_file == effect_file &&
        ctx.entries_[i].layer == layer) {
      return &ctx.entries_[i];
    }
  }
  if (ctx.entry_count_ >= kMaxEntries) return nullptr;
  Entry* entry = &ctx.entries_[ctx.entry_count_++];
  Reset(entry, effect_file, layer);
  return entry;
}

bool EffectStyles::Tint(u32 effect_file, const Color8& multiplier,
                          u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->color_mode = ColorMode::kTint;
  entry->color_first = ToUnit(multiplier);
  return true;
}

bool EffectStyles::Recolor(u32 effect_file, const Color8& start,
                             const Color8& middle, const Color8& end,
                             u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->color_mode = ColorMode::kRecolor;
  entry->color_first = ToUnit(start);
  entry->color_second = ToUnit(middle);
  entry->color_third = ToUnit(end);
  return true;
}

bool EffectStyles::ScaleLife(u32 effect_file, f32 factor, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_life = true;
  entry->life_scale = factor;
  return true;
}

bool EffectStyles::ScaleDensity(u32 effect_file, f32 factor, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_density = true;
  entry->density_scale = factor;
  return true;
}

bool EffectStyles::ScaleSize(u32 effect_file, f32 factor, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_size = true;
  entry->size_scale = factor;
  return true;
}

bool EffectStyles::Fade(u32 effect_file, f32 fade_in_end,
                          f32 fade_out_start, f32 peak, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_fade = true;
  entry->fade_in_end = Clamp(fade_in_end, 0.0f, 1.0f);
  entry->fade_out_start = Clamp(fade_out_start, entry->fade_in_end, 1.0f);
  entry->fade_peak = peak;
  return true;
}

bool EffectStyles::Disable(u32 effect_file, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_disable = true;
  return true;
}

bool EffectStyles::Loop(u32 effect_file, u32 layer) {
  Entry* entry = Acquire(effect_file, layer);
  if (entry == nullptr) return false;
  entry->has_loop = true;
  return true;
}

void EffectStyles::Remove(u32 effect_file) {
  auto& ctx = GetInstance();
  u32 i = 0;
  while (i < ctx.entry_count_) {
    if (ctx.entries_[i].effect_file == effect_file) {
      ctx.entries_[i] = ctx.entries_[ctx.entry_count_ - 1];
      ctx.entry_count_--;
    } else {
      i++;
    }
  }
}

void EffectStyles::Clear() {
  GetInstance().entry_count_ = 0;
}

bool EffectStyles::Has(u32 effect_file) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.entry_count_; i++) {
    if (ctx.entries_[i].effect_file == effect_file) return true;
  }
  return false;
}

bool EffectStyles::Resolve(u32 effect_file, ResolvedEntries* resolved) {
  auto& ctx = GetInstance();
  resolved->has_all = false;
  for (u32 p = 0; p < kMaxLayers; p++) resolved->has_layer[p] = false;
  bool found = false;
  for (u32 i = 0; i < ctx.entry_count_; i++) {
    const Entry& entry = ctx.entries_[i];
    if (entry.effect_file != effect_file) continue;
    found = true;
    if (entry.layer == kAllLayers) {
      resolved->all = entry;
      resolved->has_all = true;
    } else {
      resolved->layers[entry.layer] = entry;
      resolved->has_layer[entry.layer] = true;
    }
  }
  return found;
}

EffectStyles::Entry EffectStyles::Combine(const ResolvedEntries& resolved,
                                              u32 layer) {
  Entry combined;
  if (resolved.has_all) {
    combined = resolved.all;
  } else {
    Reset(&combined, 0, kAllLayers);
  }
  if (layer >= kMaxLayers || !resolved.has_layer[layer]) return combined;

  const Entry& specific = resolved.layers[layer];
  if (specific.color_mode != ColorMode::kNone) {
    combined.color_mode = specific.color_mode;
    combined.color_first = specific.color_first;
    combined.color_second = specific.color_second;
    combined.color_third = specific.color_third;
  }
  if (specific.has_life) {
    combined.has_life = true;
    combined.life_scale = specific.life_scale;
  }
  if (specific.has_density) {
    combined.has_density = true;
    combined.density_scale = specific.density_scale;
  }
  if (specific.has_size) {
    combined.has_size = true;
    combined.size_scale = specific.size_scale;
  }
  if (specific.has_fade) {
    combined.has_fade = true;
    combined.fade_in_end = specific.fade_in_end;
    combined.fade_out_start = specific.fade_out_start;
    combined.fade_peak = specific.fade_peak;
  }
  if (specific.has_loop) combined.has_loop = true;
  if (specific.has_disable) combined.has_disable = true;
  return combined;
}

void EffectStyles::TrackRead(const core::ArchiveInput* input) {
  auto& ctx = GetInstance();
  if (ctx.entry_count_ == 0 || input == nullptr) return;
  if (input->archive_id != ArchiveId::kMoveEffectParticle) return;
  if (input->buffer == 0) return;

  for (u32 i = 0; i < ctx.read_count_; i++) {
    if (ctx.reads_[i].destination == input->buffer) {
      ctx.reads_[i].effect_file = input->file_id;
      return;
    }
  }

  u32 slot = ctx.read_count_;
  if (slot >= kMaxTrackedReads) {
    slot = ctx.next_read_;
    ctx.next_read_ = (ctx.next_read_ + 1) % kMaxTrackedReads;
  } else {
    ctx.read_count_++;
  }
  ctx.reads_[slot].destination = input->buffer;
  ctx.reads_[slot].effect_file = input->file_id;
}

bool EffectStyles::FindFileOf(uptr resource_buffer, u32* effect_file) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.read_count_; i++) {
    u32 loaded = 0;
    SAFE_READ32(ctx.reads_[i].destination, loaded);
    if (loaded == resource_buffer) {
      *effect_file = ctx.reads_[i].effect_file;
      return true;
    }
  }
  return false;
}

bool EffectStyles::SameString(uptr first, uptr second, uptr end) {
  while (first < end && second < end) {
    const u8 a = READ8(first);
    const u8 b = READ8(second);
    if (a != b) return false;
    if (a == 0) return true;
    first++;
    second++;
  }
  return false;
}

void EffectStyles::Apply(uptr resource_buffer) {
  auto& ctx = GetInstance();
  if (ctx.entry_count_ == 0 || resource_buffer == 0) return;
  if (READ32(resource_buffer) != kGraphicsFileMagic) return;

  u32 effect_file = 0;
  if (!FindFileOf(resource_buffer, &effect_file)) return;

  ResolvedEntries resolved;
  if (!Resolve(effect_file, &resolved)) return;
  PatchBuffer(resource_buffer, resolved);
}

void EffectStyles::PatchBuffer(uptr buffer, const ResolvedEntries& resolved) {
  const u32 file_size = READ32(buffer + kFileSizeOffset);
  if (file_size < kSetMinimumSize || file_size > kMaxGraphicsFileSize) return;
  const uptr end = buffer + file_size;

  uptr emitters[kMaxEmitters];
  u32 known_emitters = 0;
  for (u32 offset = 0; offset + kEmitterMinimumSize <= file_size; offset += 4) {
    if (READ32(buffer + offset + kSignatureOffset) != kEmitterSignature) {
      continue;
    }
    if (known_emitters >= kMaxEmitters) break;
    emitters[known_emitters++] = buffer + offset;
  }

  for (u32 i = 0; i < known_emitters; i++) {
    PatchEmitter(emitters[i], Combine(resolved, i < kMaxLayers ? i : kAllLayers));
  }

  for (u32 offset = 0; offset + kSetMinimumSize <= file_size; offset += 4) {
    if (READ32(buffer + offset + kSignatureOffset) != kSetSignature) continue;
    const uptr set = buffer + offset;
    const uptr set_name = set + kNameOffset + (s32)READ32(set + kNameOffset);

    u32 layer = kAllLayers;
    for (u32 i = 0; i < known_emitters; i++) {
      const uptr path_field = emitters[i] + kEmitterSetPathOffset;
      const uptr path = path_field + (s32)READ32(path_field);
      if (path > buffer && path < end && set_name > buffer && set_name < end &&
          SameString(set_name, path, end)) {
        layer = i < kMaxLayers ? i : kAllLayers;
        break;
      }
    }
    PatchSet(buffer, set, Combine(resolved, layer));
  }
}

void EffectStyles::PatchEmitter(uptr emitter, const Entry& entry) {
  if (entry.has_loop) WRITE8(emitter + kEmitterSpanInfiniteOffset, 1);
  if (entry.has_disable) {
    WRITEF(emitter + kEmitterRatioOffset, 0.0f);
    WRITE8(emitter + kEmitterPatchedOffset, kPatchedMarker);
    return;
  }
  if (!entry.has_density) return;
  if (READ8(emitter + kEmitterPatchedOffset) == kPatchedMarker) return;
  WRITEF(emitter + kEmitterRatioOffset,
         READF(emitter + kEmitterRatioOffset) * entry.density_scale);
  WRITE8(emitter + kEmitterPatchedOffset, kPatchedMarker);
}

void EffectStyles::PatchSetCollection(uptr set, uptr collection,
                                        const Entry& entry) {
  u8 patched = READ8(set + kSetPatchedOffset);

  if (entry.has_density && entry.density_scale > 1.0f &&
      (patched & kSetCapacityPatched) == 0) {
    const f32 raw = (f32)(s32)READ32(collection + kCollectionCapacityOffset) *
                    entry.density_scale;
    s32 capacity = (s32)raw;
    if ((f32)capacity < raw) capacity++;
    WRITE32(collection + kCollectionCapacityOffset, (u32)capacity);
    patched |= kSetCapacityPatched;
  }

  if (entry.has_life && (patched & kSetLifePatched) == 0) {
    const u32 attribute_count =
        READ32(collection + kCollectionAttributeCountOffset);
    if (attribute_count <= kMaxAttributes) {
      const uptr table_field = collection + kCollectionAttributeTableOffset;
      const uptr table = table_field + (s32)READ32(table_field);
      for (u32 i = 0; i < attribute_count; i++) {
        const uptr field = table + 4 * i;
        const uptr attribute = field + (s32)READ32(field);
        if ((s32)READ32(attribute + kAttributeUsageOffset) != (s32)kLifeTarget) {
          continue;
        }
        if (READ32(attribute + kAttributeTypeOffset) != kParameterAttributeType) {
          continue;
        }
        const u32 values = READ32(attribute + kAttributeCountOffset) *
                           READ32(attribute + kAttributeDimensionOffset);
        if (values == 0 || values > kMaxParameterValues) continue;
        const uptr data_field = attribute + kAttributeTableOffset;
        const uptr data = data_field + (s32)READ32(data_field);
        for (u32 k = 0; k < values; k++) {
          WRITEF(data + 4 * k, READF(data + 4 * k) * entry.life_scale);
        }
      }
    }
    patched |= kSetLifePatched;
  }

  WRITE8(set + kSetPatchedOffset, patched);
}

void EffectStyles::PatchSet(uptr buffer, uptr set, const Entry& entry) {
  if (entry.color_mode == ColorMode::kNone && !entry.has_life &&
      !entry.has_density && !entry.has_size && !entry.has_fade) {
    return;
  }

  const uptr end = buffer + READ32(buffer + kFileSizeOffset);
  auto in_bounds = [&](uptr address, u32 size) {
    return address >= buffer && address + size <= end;
  };

  const u32 initializer_count = READ32(set + kSetInitializerCountOffset);
  const u32 updater_count = READ32(set + kSetUpdaterCountOffset);
  if (initializer_count > kMaxSetEntries) return;
  if (updater_count > kMaxSetEntries) return;

  const uptr initializer_field = set + kSetInitializerTableOffset;
  const uptr updater_field = set + kSetUpdaterTableOffset;
  const uptr collection_field = set + kSetCollectionOffset;
  if (!in_bounds(initializer_field, 4) || !in_bounds(updater_field, 4)) return;
  if (!in_bounds(collection_field, 4)) return;
  const uptr initializers = initializer_field + (s32)READ32(initializer_field);
  const uptr updaters = updater_field + (s32)READ32(updater_field);
  const uptr collection = collection_field + (s32)READ32(collection_field);
  if (!in_bounds(initializers, initializer_count * 4)) return;
  if (!in_bounds(updaters, updater_count * 4)) return;
  if (!in_bounds(collection, 12)) return;

  uptr initializer_objects[kMaxSetEntries];
  uptr updater_objects[kMaxSetEntries];
  for (u32 i = 0; i < initializer_count; i++) {
    const uptr field = initializers + 4 * i;
    initializer_objects[i] = field + (s32)READ32(field);
    if (!in_bounds(initializer_objects[i], 24)) return;
    if (READ8(initializer_objects[i] + kObjectEnabledOffset) > 1) return;
  }
  bool has_extended_scale = false;
  for (u32 i = 0; i < updater_count; i++) {
    const uptr field = updaters + 4 * i;
    updater_objects[i] = field + (s32)READ32(field);
    if (!in_bounds(updater_objects[i], 16)) return;
    if (READ8(updater_objects[i] + kObjectEnabledOffset) > 1) return;
    if (READ32(updater_objects[i] + kObjectTargetOffset) ==
        kScaleExtendedTarget) {
      has_extended_scale = true;
    }
  }
  const u32 size_target = has_extended_scale ? kScaleExtendedTarget
                                             : kScaleTarget;

  PatchSetCollection(set, collection, entry);
  for (u32 i = 0; i < initializer_count; i++) {
    PatchInitializer(initializer_objects[i], entry, size_target);
  }
  for (u32 i = 0; i < updater_count; i++) {
    if (!in_bounds(updater_objects[i], kKeyedVectorSize)) continue;
    PatchUpdater(updater_objects[i], entry, size_target);
  }
}

void EffectStyles::PatchInitializer(uptr initializer, const Entry& entry,
                                      u32 size_target) {
  if (READ8(initializer + kObjectPatchedOffset) == kPatchedMarker) return;
  const u32 target = READ32(initializer + kObjectTargetOffset);
  const u32 type = READ32(initializer + kObjectTypeOffset);

  if (target == kColorTarget && type == kColorInitializerType &&
      entry.color_mode != ColorMode::kNone) {
    const uptr value = initializer + kColorInitializerValue;
    if (entry.color_mode == ColorMode::kTint) {
      WriteVec3(value, Multiply(ReadVec3(value), entry.color_first));
    } else {
      WriteVec3(value, entry.color_first);
    }
    WRITE8(initializer + kObjectPatchedOffset, kPatchedMarker);
  } else if (target == kScaleTarget && size_target == kScaleTarget &&
             type == kScaleInitializerType && entry.has_size) {
    ScaleVec3InPlace(initializer + kScaleInitializerBase, entry.size_scale);
    WRITE8(initializer + kObjectPatchedOffset, kPatchedMarker);
  } else if (target == kLifeTarget && type == kLifeInitializerType &&
             entry.has_life) {
    WRITEF(initializer + kLifeInitializerMaximum,
           READF(initializer + kLifeInitializerMaximum) * entry.life_scale);
    WRITEF(initializer + kLifeInitializerMinimum,
           READF(initializer + kLifeInitializerMinimum) * entry.life_scale);
    WRITE8(initializer + kObjectPatchedOffset, kPatchedMarker);
  }
}

void EffectStyles::PatchUpdater(uptr updater, const Entry& entry,
                                  u32 size_target) {
  if (READ8(updater + kObjectPatchedOffset) == kPatchedMarker) return;
  const u32 target = READ32(updater + kObjectTargetOffset);

  if (target == kColorTarget && entry.color_mode != ColorMode::kNone) {
    PatchColorUpdater(updater, entry);
  } else if (target == kAlphaTarget && entry.has_fade) {
    PatchFadeUpdater(updater, entry);
  } else if (target == size_target && entry.has_size) {
    PatchSizeUpdater(updater, entry);
  }
}

void EffectStyles::PatchColorUpdater(uptr updater, const Entry& entry) {
  const u32 type = READ32(updater + kObjectTypeOffset);
  if (type == kFixedVectorUpdaterType) {
    const uptr value = updater + kFixedVectorUpdaterValue;
    if (entry.color_mode == ColorMode::kTint) {
      WriteVec3(value, Multiply(ReadVec3(value), entry.color_first));
    } else {
      WriteVec3(value, entry.color_second);
    }
    WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
    return;
  }
  if (type != kKeyedVectorUpdaterType) return;

  const f32 in_time = (f32)READ32(updater + kKeyedInTime) / kTimeScale;
  const f32 out_time = (f32)READ32(updater + kKeyedOutTime) / kTimeScale;

  Vec3 start = ReadVec3(updater + kKeyedStart);
  Vec3 middle = ReadVec3(updater + kKeyedMiddle);
  const Vec3 out_slope = ReadVec3(updater + kKeyedOutSlope);
  const Vec3 out_offset = ReadVec3(updater + kKeyedOutOffset);
  Vec3 end(out_offset.x + out_slope.x, out_offset.y + out_slope.y,
           out_offset.z + out_slope.z);

  if (entry.color_mode == ColorMode::kTint) {
    start = Multiply(start, entry.color_first);
    middle = Multiply(middle, entry.color_first);
    end = Multiply(end, entry.color_first);
  } else {
    start = entry.color_first;
    middle = entry.color_second;
    end = entry.color_third;
  }

  const f32 out_span = 1.0f - out_time;
  const Vec3 new_in_slope =
      in_time > 0.0f ? Scale(Subtract(middle, start), 1.0f / in_time) : Vec3();
  const Vec3 new_out_slope =
      out_span > 0.0f ? Scale(Subtract(end, middle), 1.0f / out_span) : Vec3();
  const Vec3 new_out_offset = Subtract(middle, Scale(new_out_slope, out_time));

  WriteVec3(updater + kKeyedStart, start);
  WriteVec3(updater + kKeyedMiddle, middle);
  WriteVec3(updater + kKeyedInSlope, new_in_slope);
  WriteVec3(updater + kKeyedOutSlope, new_out_slope);
  WriteVec3(updater + kKeyedOutOffset, new_out_offset);
  WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
}

void EffectStyles::PatchFadeUpdater(uptr updater, const Entry& entry) {
  const u32 type = READ32(updater + kObjectTypeOffset);
  if (type == kFixedNumberUpdaterType) {
    WRITEF(updater + kFixedNumberUpdaterValue, entry.fade_peak);
    WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
    return;
  }
  if (type != kKeyedNumberUpdaterType) return;

  const f32 fade_in = entry.fade_in_end;
  const f32 fade_out = entry.fade_out_start;
  const f32 peak = entry.fade_peak;

  const f32 start = fade_in > 0.0f ? 0.0f : peak;
  const f32 in_slope = fade_in > 0.0f ? peak / fade_in : 0.0f;
  const f32 out_slope = fade_out < 1.0f ? -peak / (1.0f - fade_out) : 0.0f;
  const f32 out_offset = peak - out_slope * fade_out;

  WRITE32(updater + kKeyedInTime, (u32)ToFixed(fade_in));
  WRITE32(updater + kKeyedOutTime, (u32)ToFixed(fade_out));
  WRITE32(updater + kKeyedStart, (u32)ToFixed(start));
  WRITE32(updater + kKeyedStart + 4, (u32)ToFixed(peak));
  WRITE32(updater + kKeyedStart + 8, (u32)ToFixed(in_slope));
  WRITE32(updater + kKeyedStart + 12, (u32)ToFixed(out_slope));
  WRITE32(updater + kKeyedStart + 16, (u32)ToFixed(out_offset));
  WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
}

void EffectStyles::PatchSizeUpdater(uptr updater, const Entry& entry) {
  const u32 type = READ32(updater + kObjectTypeOffset);
  if (type == kFixedVectorUpdaterType) {
    ScaleVec3InPlace(updater + kFixedVectorUpdaterValue, entry.size_scale);
    WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
  } else if (type == kKeyedVectorUpdaterType) {
    ScaleVec3InPlace(updater + kKeyedStart, entry.size_scale);
    ScaleVec3InPlace(updater + kKeyedMiddle, entry.size_scale);
    ScaleVec3InPlace(updater + kKeyedInSlope, entry.size_scale);
    ScaleVec3InPlace(updater + kKeyedOutSlope, entry.size_scale);
    ScaleVec3InPlace(updater + kKeyedOutOffset, entry.size_scale);
    WRITE8(updater + kObjectPatchedOffset, kPatchedMarker);
  }
}

}
