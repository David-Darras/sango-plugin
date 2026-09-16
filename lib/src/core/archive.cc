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

#include "core/patch/archive.h"
#include "core/hook_manager.h"
#include "renderer/native/archive/bclim.h"

namespace core {

void Archive::Initialize() {
  HookManager::Initialize(HookId::kReadFileAsync,
                          sys::address::kArchiveReadFileAsync,
                          (uptr)ReadFileAsync);
  HookManager::Initialize(HookId::kReadFileAsync2,
                          sys::address::kArchiveReadFileAsync2,
                          (uptr)ReadFileAsync2);
}

void Archive::LoadDataHook(uptr self, u32 id, uptr heap, uptr buffer,
                                uptr buffer_size, u32* size) {
  HookManager::Call<void>(HookId::kArchiveLoadData, self, id,
                          heap, buffer, buffer_size, size);
  auto* footer = (renderer::BclimFooter*)(buffer + buffer_size);
  footer--;
  if (footer->signature != 0x4D494C43) return;

  u8* p = (u8*)buffer;
  for (u32 i = 0; i < footer->pixel_data_size; i++) {
    p[i] = 0xFF;
  }
}

bool Archive::IsArchive(const u32* archive_data,
                             const ArchiveId archive_id) {
  u32* archive_table = (u32*)sys::address::kArchiveFilenameTable;
  return archive_data[12] == archive_table[static_cast<u32>(archive_id)];
}

bool Archive::IsArchive(const ArchiveInput* input, const ArchiveId archive_id) {
  return input->archive_id == archive_id;
}

bool Archive::ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                                  void* buffer,
                                  u32 p4, u32 p5, u32 p6) {
  auto& feat = GetInstance();
  if (feat.on_stream_file != nullptr) {
    file_id = feat.on_stream_file(archive, file_id);
  }
  return HookManager::Call<bool>(HookId::kReadFileAsync2, archive, heap,
                                 file_id,
                                 buffer, p4, p5, p6);
}

bool Archive::ReadFileAsync(void* file_manager, ArchiveInput* input) {
  auto& feat = GetInstance();
  if (feat.on_read_file != nullptr) feat.on_read_file(input);
  return HookManager::Call<bool>(HookId::kReadFileAsync, file_manager, input);
}

} // namespace core
