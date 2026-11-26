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
 * @file studio_save.cc
 * @brief Takes the changes of the save that Sango Studio makes while the
 * game runs.
 *
 * The declarations are in core/patch/studio_save.h.
 */

#include "core/patch/studio_save.h"

#include <cstring>

#include "core/native/data_manager.h"
#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "core/problems.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "plugin.h"
#include "savedata/native/play_time.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/savedata.h"
#include "system/native/file.h"

namespace core {
namespace {

constexpr const c16* kPath = u"sdmc:/sango-studio/save/blocks.bin";
/// "SSAV".
constexpr u32 kSignature = 0x56415353;
constexpr u32 kVersion = 1;

/// The blocks that need more than their bytes in the save.
constexpr u32 kPositionBlock = 4;
constexpr u32 kPlayTimeBlock = 6;
constexpr u32 kPartyBlock = 18;

/// One Pokémon of the block of the party: the saved data, then the calculated
/// data.
constexpr u32 kCoreSize = 232;
constexpr u32 kCalculatedSize = 28;
static_assert(sizeof(pokemon::CoreData) == kCoreSize, "size of a Pokemon");
constexpr u32 kPartyPokemonSize = kCoreSize + kCalculatedSize;
/// The block of the party: six Pokémon, then their number.
constexpr u32 kPartySize =
    savedata::PokemonTeam::kMaxSlots * kPartyPokemonSize + 4;

/// The block of the party, before it goes to its two places.
u8 party_bytes[kPartySize];

u32 Padded(u32 size) { return (size + 3) & ~3u; }

/// Opens the file of the changes. The SD card is mounted at the first miss.
bool OpenChanges(sys::File* file) {
  static bool mounted = false;
  file->Open(kPath, sys::File::kRead);
  if (file->IsOpen()) return true;
  if (!mounted) {
    mounted = true;
    sys::File::MountSdmc();
    file->Open(kPath, sys::File::kRead);
  }
  return file->IsOpen();
}

/// True when the game can take a change: the player walks freely in the world.
bool CanTakeChanges() {
  return ProcessManager::IsOverworldActive() &&
         EventManager::GetInstance().GetGameEvent() == nullptr &&
         DataManager::GetInstance().GetOverworldModelManagerOrNull() !=
             nullptr &&
         overworld::MapManager::GetInstanceOrNull() != nullptr;
}

/// Writes the Pokémon of the block of the party in the party of the player.
void WriteLiveParty() {
  auto& party = savedata::PokemonTeam::GetInstance();
  u32 count = party_bytes[savedata::PokemonTeam::kMaxSlots * kPartyPokemonSize];
  if (count > savedata::PokemonTeam::kMaxSlots) {
    count = savedata::PokemonTeam::kMaxSlots;
  }
  for (u32 i = 0; i < savedata::PokemonTeam::kMaxSlots; i++) {
    savedata::PokemonParam* pokemon = party.pokemons[i];
    if (pokemon == nullptr) return;
    const u8* bytes = party_bytes + i * kPartyPokemonSize;
    std::memcpy(pokemon->core, bytes, kCoreSize);
    std::memcpy(pokemon->runtime, bytes + kCoreSize, kCalculatedSize);
  }
  party.count = static_cast<u8>(count);
  // The game calculates the level and the statistics with its rules, which a
  // plugin can change.
  for (u32 i = 0; i < count; i++) party.pokemons[i]->UpdateRuntimeData();
}

} // namespace

void StudioSave::Load() {
#ifdef GAME_ORAS
  plugin::AddFrameListener(Update);
#endif
}

void StudioSave::Update() {
  auto& self = GetInstance();
  if (++self.frame_ % kPollFrames != 0) return;
  sys::File file;
  if (!self.is_started_) {
    // The changes of an earlier run are in the save file that the game read.
    self.is_started_ = true;
    if (OpenChanges(&file)) {
      file.Close();
      sys::File::Delete(kPath);
    }
    return;
  }
  if (!OpenChanges(&file)) return;
  Header header;
  const s32 read = file.Read(&header, sizeof(header));
  file.Close();
  if (read != static_cast<s32>(sizeof(header))) return;
  if (header.signature != kSignature || header.version != kVersion) return;
  // The file waits until the game can take it.
  if (!CanTakeChanges()) return;
  // All the blocks are good before the first one is written.
  if (ReadBlocks(header, false)) {
    ReadBlocks(header, true);
    if (header.moves_player != 0) MovePlayer(header);
  } else {
    Problems::Add(u"studio: the changes of the save are wrong");
  }
  sys::File::Delete(kPath);
}

bool StudioSave::ReadBlocks(const Header& header, bool write) {
  auto& save = savedata::SaveData::GetInstance();
  if (header.block_count > savedata::SaveData::kSegmentCount) return false;
  sys::File file;
  if (!OpenChanges(&file)) return false;
  Header again;
  if (file.Read(&again, sizeof(again)) != static_cast<s32>(sizeof(again))) {
    return false;
  }
  // The bytes to jump over before the next read.
  s64 skip = 0;
  for (u32 i = 0; i < header.block_count; i++) {
    u32 entry[2];
    if (file.Read(entry, sizeof(entry), skip) !=
        static_cast<s32>(sizeof(entry))) {
      return false;
    }
    const u32 block = entry[0];
    const u32 size = entry[1];
    // The place of the player goes through the move, not through its block.
    if (block == kPositionBlock) return false;
    u32 expected = 0;
    u8* data = save.GetSegmentData(block, &expected);
    if (data == nullptr || size != expected) return false;
    if (write) {
      WriteBlock(&file, block, data, size, 0);
      skip = Padded(size) - size;
    } else {
      // The last byte of the block is in the file.
      u8 last = 0;
      if (file.Read(&last, 1, size - 1) != 1) return false;
      skip = Padded(size) - size;
    }
  }
  return true;
}

void StudioSave::WriteBlock(sys::File* file, u32 block, u8* data, u32 size,
                            s64 skip) {
  if (block == kPartyBlock) {
    if (size != kPartySize ||
        file->Read(party_bytes, size, skip) != static_cast<s32>(size)) {
      return;
    }
    std::memcpy(data, party_bytes, size);
    WriteLiveParty();
    return;
  }
  if (file->Read(data, size, skip) != static_cast<s32>(size)) return;
  if (block == kPlayTimeBlock) {
    // The game counts the time in its own object, and saves that one.
    auto& play_time = savedata::PlayTime::GetInstance();
    std::memcpy(&play_time.hour, data, size);
  }
}

void StudioSave::MovePlayer(const Header& header) {
  // Keeps the other values of the position of the player and changes only the
  // point.
  overworld::Position position =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  position.coords.x = header.x;
  position.coords.y = header.y;
  position.coords.z = header.z;
  overworld::MapManager::ChangeMap(
      static_cast<overworld::MapId>(header.map), position,
      static_cast<overworld::Facing>(header.facing & 3), false, true);
}

} // namespace core
