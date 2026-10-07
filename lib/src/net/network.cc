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

#include "net/network.h"

#include <3ds/result.h>
#include <3ds/services/soc.h>
#include <3ds/srv.h>
#include <3ds/svc.h>
#include <3ds/thread.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>

#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "system/native/file.h"

static const c16* kStatusFilename =
    u"sdmc:/luma/plugins/000400000011C500/net_status.log";
static const c16* kConfigFilename =
    u"sdmc:/luma/plugins/000400000011C500/net.txt";

static constexpr int kHeapIdNetwork = 13;

static constexpr u32 kSocketBufferSize = 0x20000;
static constexpr u32 kWorkerStackSize = 0x8000;
static constexpr s32 kWorkerPriority = 0x30;
static constexpr int kPollTimeoutMs = 20;

static constexpr u32 kPoseIntervalFrames = 6; 
static constexpr u32 kJoinIntervalFrames = 60;
static constexpr u32 kStatusIntervalFrames = 300;
static constexpr u32 kWorldTimeoutFrames = 180;

namespace net {

bool Network::Ring::Push(const void* data, u16 size) {
  const u32 head_now = head;
  const u32 next = (head_now + 1) % kSlots;
  if (next == tail) return false;
  entries[head_now].size = size;
  memcpy(entries[head_now].data, data, size);
  __sync_synchronize();
  head = next;
  return true;
}

bool Network::Ring::Pop(Entry& out) {
  const u32 tail_now = tail;
  if (tail_now == head) return false;
  __sync_synchronize();
  out.size = entries[tail_now].size;
  memcpy(out.data, entries[tail_now].data, out.size);
  __sync_synchronize();
  tail = (tail_now + 1) % kSlots;
  return true;
}

void Network::WorkerMain(void*) {
  Network& self = GetInstance();

  srvInit();
  Result rc = socInit((u32*)self.socket_buffer_, kSocketBufferSize);
  if (R_FAILED(rc)) {
    self.worker_result_ = (s32)rc;
    self.state_ = State::kFailed;
    return;
  }

  const int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (fd < 0) {
    self.worker_result_ = -errno;
    self.state_ = State::kFailed;
    socExit();
    return;
  }

  sockaddr_in target = {};
  target.sin_family = AF_INET;
  target.sin_port = htons(self.target_port_);
  target.sin_addr.s_addr = htonl(self.target_ip_);

  self.state_ = State::kRunning;

  Ring::Entry entry;
  for (;;) {
    while (self.outgoing_.Pop(entry)) {
      sendto(fd, entry.data, entry.size, 0, (sockaddr*)&target,
             sizeof(target));
    }

    pollfd pfd = {fd, POLLIN, 0};
    if (poll(&pfd, 1, kPollTimeoutMs) <= 0 || !(pfd.revents & POLLIN)) continue;
    do {
      const ssize_t size = recvfrom(fd, entry.data, sizeof(entry.data), 0,
                                    nullptr, nullptr);
      if (size <= 0) break;
      entry.size = (u16)size;
      if (!self.incoming_.Push(entry.data, entry.size)) {
        ++self.dropped_count_;
      }
      pfd.revents = 0;
    } while (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN));
  }
}

void Network::Start() {
  state_ = State::kStarting;

  c8 text[64] = {};
  if (sys::File::ReadAll(kConfigFilename, text, sizeof(text) - 1) > 0) {
    unsigned a, b, c, d, port;
    c8 name[wire::kNameLength] = {};
    const int fields =
        sscanf(text, "%u.%u.%u.%u %u %15s", &a, &b, &c, &d, &port, name);
    if (fields >= 5) {
      target_ip_ = (a << 24) | (b << 16) | (c << 8) | d;
      target_port_ = (u16)port;
    }
    if (fields == 6) memcpy(name_, name, sizeof(name_));
  }

  void* heap = ((void* (*)(int))sys::address::kGetHeapById)(kHeapIdNetwork);
  void* raw = heap ? ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
                         heap, kSocketBufferSize + 0x1000, 4)
                   : nullptr;
  if (raw == nullptr) {
    worker_result_ = -ENOMEM;
    state_ = State::kFailed;
    return;
  }
  socket_buffer_ = (void*)(((uptr)raw + 0xFFF) & ~(uptr)0xFFF);

  if (!threadCreate(WorkerMain, nullptr, kWorkerStackSize, kWorkerPriority, -1,
                    true)) {
    worker_result_ = -EAGAIN;
    state_ = State::kFailed;
  }
}

void Network::SendHeader(wire::Type type, const void* body, u16 body_size) {
  u8 buffer[wire::kMaxDatagram];
  wire::Header header;
  header.magic = wire::kMagic;
  header.version = wire::kVersion;
  header.type = type;
  header.slot = local_slot_;
  header.sequence = sequence_++;
  memcpy(buffer, &header, sizeof(header));
  if (body_size) memcpy(buffer + sizeof(header), body, body_size);

  if (outgoing_.Push(buffer, (u16)(sizeof(header) + body_size))) {
    ++sent_count_;
  } else {
    ++dropped_count_;
  }
}

void Network::SendPose(bool is_overworld) {
  overworld::ModelManager* models =
      is_overworld ? overworld::ModelManager::GetInstanceOrNull() : nullptr;
  const bool is_changing_map =
      overworld::MapManager::GetInstance().GetNextMapId() !=
      overworld::MapManager::kNoMap;
  if (models == nullptr || is_changing_map) {
    SendHeader(wire::Type::kKeepAlive, nullptr, 0);
    return;
  }

  auto& player = models->GetPlayer();
  const u16 zone = (u16) static_cast<u32>(
      overworld::MapManager::GetInstance().GetMap());

  wire::PoseBody pose;
  pose.zone = zone;
  pose.flags = zone != last_zone_ ? wire::kPoseSnap : 0;
  last_zone_ = zone;

  const f32 turns =
      atan2f(player.facing_direction.x, player.facing_direction.z) /
          (2.0f * 3.14159265f) + 0.5f;
  pose.heading = (u8)((s32)(turns * 256.0f) & 0xFF);
  pose.x = player.draw_pos.x;
  pose.y = player.draw_pos.y;
  pose.z = player.draw_pos.z;
  SendHeader(wire::Type::kPose, &pose, sizeof(pose));
}

RemotePlayer* Network::FindOrAddRemote(u16 slot) {
  RemotePlayer* free_entry = nullptr;
  for (auto& remote : remotes_) {
    if (remote.is_active && remote.slot == slot) return &remote;
    if (!remote.is_active && free_entry == nullptr) free_entry = &remote;
  }
  return free_entry;
}

void Network::OnWelcome(const wire::WelcomeBody& body) {
  local_slot_ = body.slot;
  if (local_slot_ == 0) {
    last_join_frame_ = 0;
    return;
  }
  outfit_ = body.outfit;
  memcpy(name_, body.name, sizeof(name_));
  name_[sizeof(name_) - 1] = 0;
}

void Network::OnWorld(const u8* body, u32 size) {
  if (size < sizeof(wire::WorldBody)) return;
  const u32 count = body[0];
  if (size < sizeof(wire::WorldBody) + count * sizeof(wire::WorldEntry)) return;

  const u8* cursor = body + sizeof(wire::WorldBody);
  for (u32 i = 0; i < count; ++i, cursor += sizeof(wire::WorldEntry)) {
    wire::WorldEntry entry;
    memcpy(&entry, cursor, sizeof(entry));
    if (entry.slot == local_slot_) continue;

    RemotePlayer* remote = FindOrAddRemote(entry.slot);
    if (remote == nullptr) continue;
    remote->is_active = true;
    remote->slot = entry.slot;
    remote->zone = entry.pose.zone;
    remote->outfit = entry.outfit;
    remote->flags = entry.pose.flags;
    remote->heading = entry.pose.heading;
    remote->position.x = entry.pose.x;
    remote->position.y = entry.pose.y;
    remote->position.z = entry.pose.z;
    remote->last_seen_frame = frame_;
    memcpy(remote->name, entry.name, wire::kNameLength);
    remote->name[wire::kNameLength] = '\0';
  }

  for (auto& remote : remotes_) {
    if (remote.is_active && remote.last_seen_frame != frame_) {
      remote.is_active = false;
    }
  }
  last_world_frame_ = frame_;
}

void Network::Receive() {
  Ring::Entry entry;
  while (incoming_.Pop(entry)) {
    if (entry.size < sizeof(wire::Header)) continue;
    wire::Header header;
    memcpy(&header, entry.data, sizeof(header));
    if (header.magic != wire::kMagic || header.version != wire::kVersion) {
      continue;
    }
    ++received_count_;

    const u8* body = entry.data + sizeof(header);
    const u32 body_size = entry.size - sizeof(header);
    if (header.type == wire::Type::kWelcome &&
        body_size >= sizeof(wire::WelcomeBody)) {
      wire::WelcomeBody welcome;
      memcpy(&welcome, body, sizeof(welcome));
      OnWelcome(welcome);
    } else if (header.type == wire::Type::kWorld) {
      if (has_world_sequence_ &&
          (s16)(header.sequence - last_world_sequence_) <= 0) {
        continue;
      }
      has_world_sequence_ = true;
      last_world_sequence_ = header.sequence;
      OnWorld(body, body_size);
    }
  }
}

void Network::WriteStatus() {
  static const c8* const kStateNames[] = {"idle", "starting", "running",
                                          "failed"};
  c8 text[1024];
  int length = snprintf(
      text, sizeof(text),
      "state    : %s\nresult   : 0x%08lX\nrelay    : %u.%u.%u.%u:%u as %s\n"
      "slot     : %u (outfit %u)\nsent     : %lu\nreceived : %lu\ndropped  : %lu\n"
      "players  :\n",
      kStateNames[(u32)state_], (unsigned long)worker_result_,
      (target_ip_ >> 24) & 0xFF, (target_ip_ >> 16) & 0xFF,
      (target_ip_ >> 8) & 0xFF, target_ip_ & 0xFF, target_port_, name_,
      local_slot_, outfit_, (unsigned long)sent_count_, (unsigned long)received_count_,
      (unsigned long)dropped_count_);

  for (const auto& remote : remotes_) {
    if (!remote.is_active || length >= (int)sizeof(text)) continue;
    length += snprintf(text + length, sizeof(text) - length,
                       "  #%u %s outfit %u zone %u at %.1f %.1f %.1f\n",
                       remote.slot, remote.name, remote.outfit, remote.zone, remote.position.x,
                       remote.position.y, remote.position.z);
  }
  if (length > (int)sizeof(text)) length = sizeof(text);

  sys::File file(kStatusFilename, true);
  if (file.IsOpen()) file.Write(text, (u32)length);
}

void Network::Tick() {
  ++frame_;
  Receive();

  const bool is_overworld = core::ProcessManager::IsOverworldActive();

  if (local_slot_ == 0) {
    if (last_join_frame_ == 0 || frame_ - last_join_frame_ >= kJoinIntervalFrames) {
      wire::JoinBody join = {};
      memcpy(join.name, name_, sizeof(join.name));
      SendHeader(wire::Type::kJoin, &join, sizeof(join));
      last_join_frame_ = frame_;
    }
  } else if (frame_ - last_pose_frame_ >= kPoseIntervalFrames) {
    SendPose(is_overworld);
    last_pose_frame_ = frame_;
  }

  if (last_world_frame_ != 0 && frame_ - last_world_frame_ > kWorldTimeoutFrames) {
    for (auto& remote : remotes_) remote.is_active = false;
    last_world_frame_ = 0;
  }

  if (frame_ - last_status_frame_ >= kStatusIntervalFrames) {
    WriteStatus();
    last_status_frame_ = frame_;
  }
}

void Network::Update() {
  Network& self = GetInstance();
  switch (self.state_) {
    case State::kIdle:
      if (core::ProcessManager::IsOverworldActive()) self.Start();
      break;
    case State::kStarting:
      break;
    case State::kRunning:
      self.Tick();
      break;
    case State::kFailed: {
      static bool reported = false;
      if (!reported) {
        self.WriteStatus();
        reported = true;
      }
      break;
    }
  }
}

} // namespace net
