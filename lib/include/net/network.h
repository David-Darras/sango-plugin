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
 * @file network.h
 * @brief The online mode: other players walk in the overworld.
 *
 * The plugin sends the position of the player to a relay server
 * (tools/relay.py) and receives the positions of the other players.
 */

#pragma once

#include "common.h"
#include "net/wire.h"

namespace net {

/// Another player that the relay server sends.
struct RemotePlayer {
  bool is_active;
  u16 slot;
  u16 zone;
  u8 outfit;
  u8 flags;
  u8 heading;
  Vec3 position;
  u32 last_seen_frame;
  c8 name[wire::kNameLength + 1];
};

/// Sends the position of the player and receives the other players (16 at most).
class Network {
  MAKE_SINGLETON(Network)

public:
  static constexpr u32 kMaxRemotePlayers = 16;

  /// Sends and receives. Called one time for each frame.
  static void Update();

  INLINE bool IsConnected() const { return local_slot_ != 0; }
  INLINE u16 GetLocalSlot() const { return local_slot_; }
  INLINE u8 GetLocalOutfit() const { return outfit_; }
  INLINE const RemotePlayer* GetRemotePlayers() const { return remotes_; }

private:
  enum class State : u8 { kIdle, kStarting, kRunning, kFailed };

  struct Ring {
    static constexpr u32 kSlots = 16;
    struct Entry {
      u16 size;
      u8 data[wire::kMaxDatagram];
    };
    Entry entries[kSlots];
    volatile u32 head;
    volatile u32 tail;
    bool Push(const void* data, u16 size);
    bool Pop(Entry& out);
  };

  static void WorkerMain(void* arg);
  void Start();
  void Tick();
  void SendHeader(wire::Type type, const void* body, u16 body_size);
  void SendPose(bool is_overworld);
  void Receive();
  void OnWelcome(const wire::WelcomeBody& body);
  void OnWorld(const u8* body, u32 size);
  RemotePlayer* FindOrAddRemote(u16 slot);
  void WriteStatus();

  volatile State state_ = State::kIdle;
  volatile s32 worker_result_ = 0;

  Ring outgoing_;
  Ring incoming_;

  void* socket_buffer_ = nullptr;
  u32 target_ip_ = 0x7F000001;
  u16 target_port_ = 5000;
  c8 name_[wire::kNameLength] = {};
  u8 outfit_ = 0;

  u16 local_slot_ = 0;
  u16 sequence_ = 0;
  u32 frame_ = 0;
  u32 last_join_frame_ = 0;
  u32 last_pose_frame_ = 0;
  u32 last_status_frame_ = 0;
  u32 last_world_frame_ = 0;
  u16 last_world_sequence_ = 0;
  bool has_world_sequence_ = false;
  u32 sent_count_ = 0;
  u32 received_count_ = 0;
  u32 dropped_count_ = 0;
  u16 last_zone_ = 0xFFFF;
  RemotePlayer remotes_[kMaxRemotePlayers] = {};
};

} // namespace net
