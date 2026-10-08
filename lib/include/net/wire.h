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
 * @file wire.h
 * @brief The format of the messages between the plugin and the relay server.
 */

#pragma once

#include "core/types.h"

namespace net::wire {

constexpr u16 kMagic = 0x4753;
constexpr u8 kVersion = 2;
constexpr u32 kOutfitCount = 4;
constexpr u32 kMaxDatagram = 1024;
constexpr u32 kNameLength = 16;
constexpr u32 kMaxWorldEntries = 24;

enum class Type : u8 {
  kJoin = 1,
  kWelcome = 2,
  kPose = 3,
  kWorld = 4,
  kLeave = 5,
  kKeepAlive = 6,
};

enum PoseFlags : u8 {
  kPoseRunning = 1 << 0,
  kPoseSnap = 1 << 1,
};

#pragma pack(push, 1)

struct Header {
  u16 magic;
  u8 version;
  Type type;
  u16 slot;
  u16 sequence;
};

struct JoinBody {
  c8 name[kNameLength];
};

struct WelcomeBody {
  u16 slot;
  u8 outfit;
  c8 name[kNameLength];
};

struct PoseBody {
  u16 zone;
  u8 flags;
  u8 heading;
  f32 x;
  f32 y;
  f32 z;
};

struct WorldEntry {
  u16 slot;
  PoseBody pose;
  u8 outfit;
  c8 name[kNameLength];
};

struct WorldBody {
  u8 count;
};

#pragma pack(pop)

static_assert(sizeof(Header) == 8);
static_assert(sizeof(PoseBody) == 16);
static_assert(sizeof(WelcomeBody) == 19);
static_assert(sizeof(WorldEntry) == 35);
static_assert(sizeof(Header) + sizeof(WorldBody) +
                  kMaxWorldEntries * sizeof(WorldEntry) <= kMaxDatagram);

} // namespace net::wire
