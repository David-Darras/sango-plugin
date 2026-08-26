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

#pragma once

#include "archive.h"
#include "common.h"
#include "game/constant/gender.h"
#include "game/overworld/map_manager.h"
#include "game/overworld/model_manager.h"
#include "game/overworld/renderer.h"
#include "game/renderer/h3d_resource.h"
#include "game/renderer/h3d_shader_model.h"
#include "game/renderer/pokemon_model.h"
#include "game/renderer/scene.h"
#include "ui/log_application.h"

namespace feature {
struct LoadedModel {
  void* model_pack = nullptr;
  void* texture_pack = nullptr;
  renderer::H3dResource* model_resource = nullptr;
  renderer::H3dResource* texture_resource = nullptr;
  renderer::H3dShaderModel* model = nullptr;

  INLINE bool IsLoaded() const { return model != nullptr; }
};

class ModelLoader {
  MAKE_SINGLETON(ModelLoader)

public:
  static constexpr u32 kMaxLoaded = 16;

  static bool LoadOverworldModel(LoadedModel* out, u16 model_id,
                                 const Vec3& position) {
    if (out == nullptr || out->IsLoaded()) return false;

    renderer::H3dResource* shader =
        overworld::ModelManager::GetInstance().GetShaderResource();
    if (shader == nullptr) {
      ui::LogApplication::Print(u"shader unavailable");
      return false;
    }

    out->model_pack = ReadFile(ARCHIVE_OVERWORLD_MODEL, model_id, true);
    out->model_resource = AttachPackEntry(out->model_pack, 0);
    if (out->model_resource == nullptr) return false;

    void* heap_allocator = HeapAllocator();
    void* device_allocator = DeviceAllocator();
    out->model_resource->Setup(heap_allocator, device_allocator, shader);
    out->model_resource->Setup(heap_allocator, device_allocator, nullptr,
                               renderer::H3dResource::kCommonDefaultShader);
    if (!out->model_resource->IsSetup()) {
      ReportUnresolved(out->model_resource);
      return false;
    }

    if (!BuildModel(out, position)) return false;
    out->model->ApplyOverworldNpcDefaults();
    return true;
  }

  static bool LoadPokemon(LoadedModel* out, u16 species, u8 form,
                          bool is_shiny, const Vec3& position,
                          u8 gender = GENDER_MALE) {
    if (out == nullptr || out->IsLoaded()) return false;

    PokeModelTable table(GetPokemonTable());
    if (!table.IsValid()) return false;

    const u32 pack_top = table.GetPackTop(species, form, gender);
    if (pack_top == PokeModelTable::kInvalidPack) {
      ui::LogApplication::Print(u"unknown species %d", species);
      return false;
    }

    const u32 texture_slot =
        is_shiny
          ? POKEMON_FILE_SECTION_TEXTURE_SHINY
          : POKEMON_FILE_SECTION_TEXTURE_NORMAL;

    out->model_pack =
        ReadFile(ARCHIVE_POKEMON_MODEL, pack_top + POKEMON_FILE_SECTION_COMMON,
                 true);
    out->texture_pack =
        ReadFile(ARCHIVE_POKEMON_MODEL, pack_top + texture_slot, true);

    out->model_resource = AttachPackEntry(out->model_pack, 0);
    out->texture_resource = AttachPackEntry(out->texture_pack, 0);
    if (out->model_resource == nullptr) return false;

    void* heap_allocator = HeapAllocator();
    void* device_allocator = DeviceAllocator();

    if (out->texture_resource != nullptr) {
      out->texture_resource->Setup(heap_allocator, device_allocator);
      out->model_resource->Setup(heap_allocator, device_allocator,
                                 out->texture_resource);
    }
    LinkPokemonCommonResources(out->model_resource);
    out->model_resource->Setup(heap_allocator, device_allocator, nullptr,
                               renderer::H3dResource::kCommonDefaultShader);
    if (!out->model_resource->IsSetup()) {
      ReportUnresolved(out->model_resource);
      return false;
    }

    return BuildModel(out, position);
  }

  static void DropAll() {
    void* scene = Scene();
    if (scene == nullptr) return;
    auto& context = GetInstance();
    for (u32 i = 0; i < kMaxLoaded; ++i) {
      LoadedModel* entry = context.loaded_[i];
      if (entry == nullptr) continue;
      if (entry->model != nullptr && scene != nullptr) {
        renderer::Scene::Unregister(scene, entry->model);
        entry->model->Destroy();
        entry->model_resource->RemoveData();
        entry->texture_resource->RemoveData();
      }
      *entry = LoadedModel{};
      context.loaded_[i] = nullptr;
    }
    context.loaded_count_ = 0;
    context.pokemon_table_ = nullptr;
    context.pokemon_common_count_ = 0;
  }

private:
  LoadedModel* loaded_[kMaxLoaded] = {};
  u32 loaded_count_ = 0;
  void* pokemon_table_ = nullptr;
  renderer::H3dResource* pokemon_common_[8] = {};
  u32 pokemon_common_count_ = 0;

  STATIC_INLINE void* Heap() {
    return overworld::MapManager::GetInstance().GetHeap();
  }

  STATIC_INLINE void* DeviceHeap() {
    return overworld::MapManager::GetInstance().GetDeviceHeap();
  }

  STATIC_INLINE void* HeapAllocator() {
    return overworld::MapManager::GetInstance().GetHeapAllocator();
  }

  STATIC_INLINE void* DeviceAllocator() {
    return overworld::MapManager::GetInstance().GetDeviceAllocator();
  }

  STATIC_INLINE void* Scene() {
    if (game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE)) {
      auto* renderer = overworld::Renderer::GetInstance();
      if (renderer == nullptr) return nullptr;
      return renderer->GetScene();
    }
    return nullptr;
  }

  static void* OpenArchive(u32 archive_id) {
    static void* archives[256] = {};
    if (archive_id >= 256) return nullptr;
    if (archives[archive_id] != nullptr) return archives[archive_id];

    void* heap = DeviceHeap();
    if (heap == nullptr) return nullptr;

    constexpr u32 kSizeOfArcFile = 120;
    constexpr u32 kOpen = 1 << 0;
    void* archive = ((void* (*)(void*, u32, u32))ADDRESS_HEAP_ALLOC)(
        heap, kSizeOfArcFile, 4);
    if (archive == nullptr) return nullptr;
    ((void (*)(void*, void*, u32, u32))ADDRESS_ARCHIVE_INITIALIZE)(
        archive, heap, archive_id, kOpen);
    archives[archive_id] = archive;
    return archive;
  }

  static void* ReadFile(u32 archive_id, u32 file_id, bool compressed,
                        u32* out_size = nullptr) {
    void* archive = OpenArchive(archive_id);
    if (archive == nullptr) return nullptr;

    void* device_heap = DeviceHeap();
    u32 size = 0;
    void* buffer = nullptr;

    if (compressed) {
      buffer = ((void* (*)(void*, u32, void*, void*, s32, u32*))
        ADDRESS_ARCHIVE_LOAD_COMPRESSED_FILE)(archive, file_id, Heap(),
                                              device_heap, 128, &size);
    } else {
      size = ((u32 (*)(void*, u32))ADDRESS_ARCHIVE_GET_FILE_SIZE)(archive,
        file_id);
      if (size == 0) return nullptr;
      buffer = ((void* (*)(void*, u32, u32))ADDRESS_HEAP_ALLOC)(
          device_heap, size, 128);
      if (buffer == nullptr) return nullptr;
      ((void (*)(void*, u32, u32, u32, void*))ADDRESS_ARCHIVE_LOAD_FILE)(
          archive, file_id, 0, size, buffer);
    }

    if (out_size != nullptr) *out_size = size;
    return buffer;
  }

  static renderer::H3dResource* AttachPackEntry(void* pack, u32 index) {
    if (pack == nullptr) return nullptr;
    auto* resource_pack = (ResourcePack*)pack;
    if (index >= resource_pack->file_count) return nullptr;

    auto* resource = renderer::H3dResource::Create(DeviceHeap());
    if (resource == nullptr) return nullptr;
    resource->SetData((void*)resource_pack->GetResource(index));
    return resource;
  }

  static void* GetPokemonTable() {
    auto& context = GetInstance();
    if (context.pokemon_table_ == nullptr) {
      context.pokemon_table_ =
          ReadFile(ARCHIVE_POKEMON_MODEL, POKEMON_FILE_SPECIES_TABLE, false);
    }
    return context.pokemon_table_;
  }

  static void LinkPokemonCommonResources(renderer::H3dResource* target) {
    auto& context = GetInstance();
    void* heap_allocator = HeapAllocator();
    void* device_allocator = DeviceAllocator();

    if (context.pokemon_common_count_ == 0) {
      void* pack =
          ReadFile(ARCHIVE_POKEMON_MODEL, POKEMON_FILE_COMMON_SHADER, false);
      if (pack == nullptr) return;
      auto* resource_pack = (ResourcePack*)pack;
      const u32 count = resource_pack->file_count;
      for (u32 i = 0; i < count; ++i) {
        if (context.pokemon_common_count_ >= 8) break;
        auto* entry = (u32*)resource_pack->GetResource(i);
        if ((entry[0] & 0xFFFFFF) != 0x484342) continue;
        auto* resource = renderer::H3dResource::Create(DeviceHeap());
        if (resource == nullptr) break;
        resource->SetData(entry);
        resource->Setup(heap_allocator, device_allocator);
        context.pokemon_common_[context.pokemon_common_count_++] = resource;
      }
    }

    for (u32 i = 0; i < context.pokemon_common_count_; ++i) {
      target->Setup(heap_allocator, device_allocator,
                    context.pokemon_common_[i]);
      if (target->IsSetup()) break;
    }
  }

  static bool BuildModel(LoadedModel* out, const Vec3& position) {
    out->model = renderer::H3dShaderModel::Create(DeviceHeap());
    if (out->model == nullptr) {
      ui::LogApplication::Print(u"model constructor failed");
      return false;
    }

    renderer::H3dShaderModel::Metadata description =
        renderer::H3dShaderModel::DefaultMetadata();
    description.anim_count = 0;
    out->model->Build(HeapAllocator(), DeviceAllocator(), out->model_resource,
                      0, &description);

    if (out->model->GetMaterialCount() <= 0) {
      ui::LogApplication::Print(u"model has no material");
      return false;
    }

    void* scene = Scene();
    if (scene == nullptr || !renderer::Scene::Register0(scene, out->model) ||
        !renderer::Scene::Register1(scene, out->model)) {
      ui::LogApplication::Print(u"scene registration refused");
      return false;
    }

    out->model->SetTranslate(position);
    Track(out);
    return true;
  }

  static void Track(LoadedModel* entry) {
    auto& context = GetInstance();
    if (context.loaded_count_ >= kMaxLoaded) return;
    context.loaded_[context.loaded_count_++] = entry;
  }

  static void ReportUnresolved(renderer::H3dResource* resource) {
    ui::LogApplication::Print(u"setup failed, unresolved %X",
                              resource->GetState());
  }
};
} // namespace feature