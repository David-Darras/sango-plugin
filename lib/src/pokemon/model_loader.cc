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
 * @file model_loader.cc
 * @brief Loads 3D models (Pokémon, characters, decorations) and shows them in the overworld.
 *
 * The declarations are in pokemon/patch/model_loader.h.
 */

#include "pokemon/patch/model_loader.h"

#include "core/constant/archive_id.h"
#include "core/native/process_manager.h"
#include "core/utils.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/renderer.h"
#include "pokemon/native/model_motion.h"
#include "pokemon/native/species_data.h"
#include "renderer/patch/lighting.h"
#include "renderer/native/h3d_model.h"
#include "renderer/native/h3d_resource.h"
#include "renderer/native/h3d_shader_model.h"
#include "renderer/native/pokemon_model_table.h"
#include "renderer/native/scene.h"
#include "system/native/file.h"
#include "core/game_file.h"
#include "ui/log.h"
#include "overworld/address.h"
#include "system/address.h"

#define POKEMON_FILE_SPECIES_TABLE (0)
#define POKEMON_FILE_COMMON_SHADER (1)

#define POKEMON_FILE_SECTION_COMMON (0)
#define POKEMON_FILE_SECTION_FACE_ANIMATION (1)
#define POKEMON_FILE_SECTION_TEXTURE_NORMAL (2)
#define POKEMON_FILE_SECTION_TEXTURE_SHINY (3)
#define POKEMON_FILE_SECTION_BATTLE_ANIMATION (5)
#define POKEMON_FILE_SECTION_POKEMON_AMIE_ANIMATION (6)
#define POKEMON_FILE_SECTION_OTHER (7)

namespace pokemon {

namespace {
constexpr s32 kPokemonLightSet = 15;
// The animations that a Pokémon plays by itself, to look alive: the numbers of
// the animations of the battles (the attacks) and of Pokémon-Amie (it answers,
// turns, is happy, plays). The animations that go with others (sleep, eat, the
// entrances) are not here.
// (The animation 16 of the battles is the damage animation: it is not here.)
const s32 kBattleActions[] = {8, 9, 10, 11, 12, 13, 14, 15};
const s32 kAmieActions[] = {1, 2, 3, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20};
constexpr u32 kQuietFramesMin = 360; // 6 seconds.
constexpr u32 kQuietFramesRandom = 540;
// The volume of the cry of the model. 1 is the normal volume of the game.
constexpr f32 kCryVolume = 2.0f;
// The light that the game made for the Pokémon belongs to a scene.
void* g_light_scene = nullptr;
void* g_light_and_set[2] = {};
// The archive objects of the game that the loader opened (by archive id).
constexpr u32 kMaxArchiveIds = 512;
void* g_archives[kMaxArchiveIds] = {};

// A short list of the last steps of the Pokémon that follows. It goes to a text
// file after each step, so the file shows the last step before a freeze.
// The writes of the file are slow on some systems (the game stops a moment
// for each write): the trace is off. Turn it on to look for a freeze.
constexpr bool kTraceEnabled = false;

void Trace(const c8* text, u32 a = 0, u32 b = 0) {
  if (!kTraceEnabled) return;
  constexpr u32 kLines = 24;
  constexpr u32 kLineSize = 64;
  static c8 lines[kLines][kLineSize];
  static u32 count = 0;
  snprintf(lines[count % kLines], kLineSize, "%lu %s %lu %lu",
           (unsigned long)count, text, (unsigned long)a, (unsigned long)b);
  count++;
  static c8 out[kLines * kLineSize];
  u32 size = 0;
  const u32 first = count > kLines ? count - kLines : 0;
  for (u32 i = first; i < count; i++) {
    size += snprintf(out + size, sizeof(out) - size, "%s\n",
                     lines[i % kLines]);
  }
  sys::File::CreateDirectory(u"sdmc:/sango");
  sys::File file(u"sdmc:/sango/follow_trace.txt", true);
  if (file.IsOpen()) file.Write(out, size);
}
} // namespace

bool ModelLoader::LoadOverworldModel(LoadedModel* out, ModelId model_id,
                                     const Vec3& position) {
  if (out == nullptr || out->IsLoaded()) return false;

  renderer::H3dResource* shader =
      overworld::ModelManager::GetInstance().GetShaderResource();
  if (shader == nullptr) {
    ui::Log::Print(u"shader unavailable");
    return false;
  }

  out->model_pack = ReadFile(ArchiveId::kOverworldModel,
                             static_cast<u32>(model_id), true);
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

bool ModelLoader::LoadPokemon(LoadedModel* out, SpeciesId species, FormId form,
                              bool is_shiny, const Vec3& position,
                              Gender gender) {
  if (out == nullptr || out->IsLoaded()) return false;

  Trace("before load heap/device KB", core::GameFile::GetFreeHeapMemory(0) / 1024,
        core::GameFile::GetFreeHeapMemory(1) / 1024);
  renderer::PokemonModelTable table(GetPokemonTable());
  if (!table.IsValid()) return false;

  const u32 pack_top = table.GetPackTop(species, form, gender);
  out->species = species;
  out->form = form;
  // A Pokémon that flies or floats does not walk.
  {
    const SpeciesData& data = SpeciesData::GetInstance(species, form);
    out->walk.can_walk = data.type[0] != TypeId::kFlying &&
                         data.type[1] != TypeId::kFlying &&
                         data.ability[0] != AbilityId::kLevitate &&
                         data.ability[1] != AbilityId::kLevitate &&
                         data.ability[2] != AbilityId::kLevitate;
  }
  if (pack_top == renderer::PokemonModelTable::kInvalidPack) {
    ui::Log::Print(u"unknown species %d", species);
    return false;
  }

  const u32 texture_slot =
      is_shiny
        ? POKEMON_FILE_SECTION_TEXTURE_SHINY
        : POKEMON_FILE_SECTION_TEXTURE_NORMAL;

  Trace("free before read heap/device KB",
        core::GameFile::GetFreeMemory(Heap()) / 1024,
        core::GameFile::GetFreeMemory(DeviceHeap()) / 1024);
  out->model_pack =
      ReadFile(ArchiveId::kPokemonModel,
               pack_top + POKEMON_FILE_SECTION_COMMON, true);
  Trace("model read heap/device KB",
        core::GameFile::GetFreeMemory(Heap()) / 1024,
        core::GameFile::GetFreeMemory(DeviceHeap()) / 1024);
  out->texture_pack =
      ReadFile(ArchiveId::kPokemonModel, pack_top + texture_slot, true);
  Trace("texture read heap/device KB",
        core::GameFile::GetFreeMemory(Heap()) / 1024,
        core::GameFile::GetFreeMemory(DeviceHeap()) / 1024);

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

  if (!BuildModel(out, position)) return false;
  AttachMotion(out, pack_top);
  return true;
}

bool ModelLoader::LoadDecoration(LoadedModel* out, u32 decoration_index,
                                 const Vec3& position) {
  if (out == nullptr || out->IsLoaded()) return false;
  if (!overworld::address::kDecorationTable ||
      decoration_index >= overworld::address::kDecorationCount) {
    return false;
  }

  const u8* info =
      reinterpret_cast<const u8*>(overworld::address::kDecorationTable) +
      decoration_index * overworld::address::kDecorationEntrySize;
  const u32 pack_id = info[17];
  const u32 model_index = info[18];

  out->model_pack = ReadFile(ArchiveId::kSecretBase, pack_id, false);
  if (out->model_pack == nullptr) {
    ui::Log::Print(u"decoration %u: pack %u unreadable",
                              decoration_index, pack_id);
    return false;
  }
  out->model_resource = AttachPackEntry(out->model_pack, model_index);
  if (out->model_resource == nullptr) {
    ui::Log::Print(u"decoration %u: model %u missing",
                              decoration_index, model_index);
    return false;
  }

  out->model_resource->Setup(HeapAllocator(), DeviceAllocator(), nullptr,
                             renderer::H3dResource::kCommonDefaultShader);
  if (!out->model_resource->IsSetup()) {
    ReportUnresolved(out->model_resource);
    return false;
  }
  return BuildModel(out, position);
}

void* ModelLoader::ReadSdPack(const c16* path, u32 capacity) {
  void* device_heap = DeviceHeap();
  if (device_heap == nullptr) return nullptr;
  void* buffer = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
      device_heap, capacity, 128);
  if (buffer == nullptr) return nullptr;

  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) {
    sys::File::MountSdmc();
    file.Open(path, sys::File::kRead);
  }
  if (!file.IsOpen()) {
    ((void (*)(void*))sys::address::kHeapFree)(buffer);
    return nullptr;
  }
  const s32 read = file.Read(buffer, capacity);
  file.Close();
  if (read <= 0) {
    ((void (*)(void*))sys::address::kHeapFree)(buffer);
    return nullptr;
  }
  return buffer;
}

renderer::H3dResource* ModelLoader::LoadPackResource(void* pack, u32 index) {
  renderer::H3dResource* resource = AttachPackEntry(pack, index);
  if (resource == nullptr) return nullptr;

  void* heap_allocator = HeapAllocator();
  void* device_allocator = DeviceAllocator();
  auto* field_renderer = overworld::Renderer::GetInstance();
  renderer::H3dResource* shader =
      field_renderer != nullptr ? field_renderer->GetShaderResource() : nullptr;
  if (shader != nullptr) resource->Setup(heap_allocator, device_allocator, shader);
  resource->Setup(heap_allocator, device_allocator, nullptr,
                  renderer::H3dResource::kCommonDefaultShader);
  if (!resource->IsSetup()) {
    ReportUnresolved(resource);
    return nullptr;
  }
  return resource;
}

bool ModelLoader::LoadShared(LoadedModel* out, renderer::H3dResource* resource,
                             const Vec3& position) {
  if (out == nullptr || out->IsLoaded() || resource == nullptr) return false;
  out->model_resource = resource;
  out->is_resource_shared = true;
  if (!BuildModel(out, position)) {
    *out = LoadedModel{};
    return false;
  }
  return true;
}

void ModelLoader::FreeBuffer(void* buffer) {
  if (buffer != nullptr) {
    ((void (*)(void*))sys::address::kHeapFree)(buffer);
  }
}

void ModelLoader::Untrack(LoadedModel* entry) {
  auto& context = GetInstance();
  for (u32 i = 0; i < context.loaded_count_; ++i) {
    if (context.loaded_[i] != entry) continue;
    context.loaded_[i] = context.loaded_[--context.loaded_count_];
    context.loaded_[context.loaded_count_] = nullptr;
    return;
  }
}

void ModelLoader::DestroyMotion(LoadedModel* entry) {
#ifdef GAME_ORAS
  if (entry->motion != nullptr) ModelMotion::Destroy(entry->motion);
  if (entry->amie_motion != nullptr) ModelMotion::Destroy(entry->amie_motion);
  entry->amie_motion = nullptr;
#endif
  entry->motion = nullptr;
}

void ModelLoader::ForgetGameMemory() {
  auto& context = GetInstance();
  context.pokemon_table_ = nullptr;
  context.pokemon_common_count_ = 0;
  for (u32 i = 0; i < kMaxArchiveIds; i++) g_archives[i] = nullptr;
  g_light_scene = nullptr;
}

void ModelLoader::Discard(LoadedModel* entry) {
  if (entry == nullptr) return;
  if (entry->IsLoaded()) Trace("discard");
  Untrack(entry);
  *entry = LoadedModel{};
  // A new scene needs a new light.
  g_light_scene = nullptr;
}

void ModelLoader::UpdatePokemonLight() {
#ifdef GAME_ORAS
  if (g_light_scene == nullptr || g_light_and_set[0] == nullptr) return;
  // The hooks of the lighting module keep the colours that the game gives to
  // the light of the overworld: they change with the time of the day.
  const Color& ambient = renderer::Lighting::GetInstance().ambient_color;
  const Color& diffuse = renderer::Lighting::GetInstance().diffuse_color;
  auto set = [](uptr function, void* light, f32 r, f32 g, f32 b) {
    ((void (*)(void*, f32, f32, f32))function)(light, r, g, b);
  };
  void* light = g_light_and_set[0];
  set(renderer::address::kLightSetAmbient, light, ambient.r, ambient.g,
      ambient.b);
  set(renderer::address::kLightSetDiffuse, light, diffuse.r, diffuse.g,
      diffuse.b);
  // The shine follows the diffuse light: it is dark at night.
  set(renderer::address::kLightSetSpecular0, light, diffuse.r * 0.5f,
      diffuse.g * 0.5f, diffuse.b * 0.5f);
  set(renderer::address::kLightSetSpecular1, light, diffuse.r * 0.5f,
      diffuse.g * 0.5f, diffuse.b * 0.5f);
#endif
}

void ModelLoader::ReleaseLight(void* scene) {
#ifdef GAME_ORAS
  if (scene == nullptr || g_light_scene != scene) return;
  auto& context = GetInstance();
  for (u32 i = 0; i < context.loaded_count_; ++i) {
    if (context.loaded_[i] != nullptr && context.loaded_[i]->motion != nullptr) {
      return; // Another Pokémon model uses the light.
    }
  }
  Trace("light: set null");
  ((void (*)(void*, s32, void*))renderer::address::kSceneSetLightSet)(
      scene, kPokemonLightSet, nullptr);
  Trace("light: remove node");
  ((void (*)(void*, void*))renderer::address::kSceneRemoveNodeFromCalc)(
      scene, g_light_and_set[0]);
  Trace("light: removed");
  g_light_scene = nullptr;
#else
  (void)scene;
#endif
}

void ModelLoader::ReleasePacks(LoadedModel* entry) {
  // Only the Pokémon models: they read four files each time. The memory of a
  // file that nobody frees stays in the heap of the map until the map ends.
  if (entry->species == SpeciesId::kNone) return;
  FreeBuffer(entry->model_pack);
  FreeBuffer(entry->texture_pack);
  FreeBuffer(entry->motion_pack);
  FreeBuffer(entry->amie_pack);
  Trace("packs freed heap/device KB",
        core::GameFile::GetFreeMemory(Heap()) / 1024,
        core::GameFile::GetFreeMemory(DeviceHeap()) / 1024);
}

void ModelLoader::Drop(LoadedModel* entry) {
  if (entry == nullptr) return;
  if (entry->IsLoaded()) Trace("drop");
  DestroyMotion(entry);
  if (entry->IsLoaded()) Trace("drop: motion destroyed");
  void* scene = Scene();
  if (entry->model != nullptr && scene != nullptr) {
    Trace("drop: unregister");
    renderer::Scene::Unregister(scene, entry->model);
    Trace("drop: destroy");
    entry->model->Destroy();
    Trace("drop: remove data");
    if (entry->model_resource != nullptr && !entry->is_resource_shared) {
      entry->model_resource->RemoveData();
    }
    if (entry->texture_resource != nullptr) {
      entry->texture_resource->RemoveData();
    }
    ReleasePacks(entry);
    Trace("drop: done");
  }
  Untrack(entry);
  *entry = LoadedModel{};
  ReleaseLight(scene);
}

void ModelLoader::DropAll() {
  void* scene = Scene();
  if (scene == nullptr) return;
  auto& context = GetInstance();
  for (u32 i = 0; i < kMaxLoaded; ++i) {
    LoadedModel* entry = context.loaded_[i];
    if (entry == nullptr) continue;
    DestroyMotion(entry);
    if (entry->model != nullptr && scene != nullptr) {
      renderer::Scene::Unregister(scene, entry->model);
      entry->model->Destroy();
      if (!entry->is_resource_shared) entry->model_resource->RemoveData();
      if (entry->texture_resource != nullptr) {
        entry->texture_resource->RemoveData();
      }
      ReleasePacks(entry);
    }
    *entry = LoadedModel{};
    context.loaded_[i] = nullptr;
  }
  context.loaded_count_ = 0;
  ReleaseLight(scene);
  context.pokemon_table_ = nullptr;
  context.pokemon_common_count_ = 0;
}

void* ModelLoader::Heap() {
  return overworld::MapManager::GetInstance().GetHeap();
}

void* ModelLoader::DeviceHeap() {
  return overworld::MapManager::GetInstance().GetDeviceHeap();
}

void* ModelLoader::HeapAllocator() {
  return overworld::MapManager::GetInstance().GetHeapAllocator();
}

void* ModelLoader::DeviceAllocator() {
  return overworld::MapManager::GetInstance().GetDeviceAllocator();
}

void* ModelLoader::Scene() {
  if (core::ProcessManager::GetInstance().IsCurrentProcess(
      overworld::address::kVtable)) {
    auto* maps = overworld::MapManager::GetInstanceOrNull();
    if (maps == nullptr) return nullptr;
    auto* renderer = maps->GetRenderer();
    if (renderer == nullptr) return nullptr;
    return renderer->GetScene();
  }
  return nullptr;
}

void* ModelLoader::OpenArchive(ArchiveId archive_id) {
  constexpr u32 kMaxArchives = kMaxArchiveIds;
  const u32 index = static_cast<u32>(archive_id);
  if (index >= kMaxArchives) return nullptr;
  if (g_archives[index] != nullptr) return g_archives[index];

  void* heap = DeviceHeap();
  if (heap == nullptr) return nullptr;

  constexpr u32 kSizeOfArcFile = 120;
  constexpr u32 kOpen = 1 << 0;
  void* archive = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
      heap, kSizeOfArcFile, 4);
  if (archive == nullptr) return nullptr;
  ((void (*)(void*, void*, u32, u32))sys::address::kArchiveInitialize)(
      archive, heap, index, kOpen);
  g_archives[index] = archive;
  return archive;
}

void* ModelLoader::ReadFile(ArchiveId archive_id, u32 file_id, bool compressed,
                            u32* out_size) {
  void* archive = OpenArchive(archive_id);
  if (archive == nullptr) return nullptr;

  void* device_heap = DeviceHeap();
  u32 size = 0;
  void* buffer = nullptr;

  if (compressed) {
    buffer = ((void* (*)(void*, u32, void*, void*, s32, u32*))
      sys::address::kArchiveLoadCompressedFile)(archive, file_id, Heap(),
                                            device_heap, 128, &size);
  } else {
    size = ((u32 (*)(void*, u32))sys::address::kArchiveGetFileSize)(archive,
      file_id);
    if (size == 0) return nullptr;
    buffer = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
        device_heap, size, 128);
    if (buffer == nullptr) return nullptr;
    ((void (*)(void*, u32, u32, u32, void*))sys::address::kArchiveLoadFile)(
        archive, file_id, 0, size, buffer);
  }

  if (out_size != nullptr) *out_size = size;
  return buffer;
}

renderer::H3dResource* ModelLoader::AttachPackEntry(void* pack, u32 index) {
  if (pack == nullptr) return nullptr;
  auto* bundle = (Bundle*)pack;
  if (index >= bundle->resource_count) return nullptr;

  auto* resource = renderer::H3dResource::Create(DeviceHeap());
  if (resource == nullptr) return nullptr;
  resource->SetData((void*)bundle->GetResource(index));
  return resource;
}

void* ModelLoader::GetPokemonTable() {
  auto& context = GetInstance();
  if (context.pokemon_table_ == nullptr) {
    context.pokemon_table_ =
        ReadFile(ArchiveId::kPokemonModel, POKEMON_FILE_SPECIES_TABLE, false);
  }
  return context.pokemon_table_;
}

void ModelLoader::LinkPokemonCommonResources(renderer::H3dResource* target) {
  auto& context = GetInstance();
  void* heap_allocator = HeapAllocator();
  void* device_allocator = DeviceAllocator();

  if (context.pokemon_common_count_ == 0) {
    void* pack =
        ReadFile(ArchiveId::kPokemonModel, POKEMON_FILE_COMMON_SHADER, false);
    if (pack == nullptr) return;
    auto* bundle = (Bundle*)pack;
    const u32 count = bundle->resource_count;
    for (u32 i = 0; i < count; ++i) {
      if (context.pokemon_common_count_ >= 8) break;
      auto* entry = (u32*)bundle->GetResource(i);
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

bool ModelLoader::BuildModel(LoadedModel* out, const Vec3& position) {
  out->model = renderer::H3dShaderModel::Create(DeviceHeap());
  if (out->model == nullptr) {
    ui::Log::Print(u"model constructor failed");
    return false;
  }

  renderer::H3dShaderModel::Metadata description =
      renderer::H3dShaderModel::DefaultMetadata();
  description.anim_count = 0;
  out->model->Build(HeapAllocator(), DeviceAllocator(), out->model_resource,
                    0, &description);

  if (out->model->GetMaterialCount() <= 0) {
    ui::Log::Print(u"model has no material");
    return false;
  }

  void* scene = Scene();
  if (scene == nullptr || !renderer::Scene::Register0(scene, out->model) ||
      !renderer::Scene::Register1(scene, out->model)) {
    ui::Log::Print(u"scene registration refused");
    return false;
  }

  out->model->SetTranslate(position);
  ApplyPokemonLight(out->model, scene);
  Track(out);
  return true;
}

void ModelLoader::AttachMotion(LoadedModel* out, u32 pack_top) {
#ifdef GAME_ORAS
  out->motion_pack = ReadFile(ArchiveId::kPokemonModel,
                              pack_top + POKEMON_FILE_SECTION_BATTLE_ANIMATION,
                              true);
  auto* bundle = static_cast<Bundle*>(out->motion_pack);
  if (bundle == nullptr || bundle->resource_count == 0) {
    ui::Log::Print(u"no animation pack");
    return;
  }
  void* heap_base = *(void**)((u8*)HeapAllocator() + 4);
  out->motion = ModelMotion::Create(
      heap_base, reinterpret_cast<void*>(bundle->GetResource(0)), out->model);
  if (out->motion == nullptr) {
    ui::Log::Print(u"animation failed");
    return;
  }
  if (!ModelMotion::Play(out->motion, 0)) ui::Log::Print(u"no idle animation");

  // The animations of Pokémon-Amie. They are not necessary.
  out->amie_pack = ReadFile(ArchiveId::kPokemonModel,
                            pack_top + POKEMON_FILE_SECTION_POKEMON_AMIE_ANIMATION,
                            true);
  auto* amie_bundle = static_cast<Bundle*>(out->amie_pack);
  if (amie_bundle != nullptr && amie_bundle->resource_count != 0) {
    out->amie_motion = ModelMotion::Create(
        heap_base, reinterpret_cast<void*>(amie_bundle->GetResource(0)),
        out->model);
    if (out->amie_motion != nullptr) ModelMotion::Play(out->amie_motion, 0);
  }
  out->next_action_frames =
      kQuietFramesMin + core::Utils::GetRandomValue(kQuietFramesRandom);
  Trace("loaded heap/device KB", core::GameFile::GetFreeHeapMemory(0) / 1024,
        core::GameFile::GetFreeHeapMemory(1) / 1024);
#else
  (void)out;
  (void)pack_top;
#endif
}

void* ModelLoader::ActiveMotion(const LoadedModel& entry) {
  return entry.is_amie && entry.amie_motion != nullptr ? entry.amie_motion
                                                      : entry.motion;
}

void ModelLoader::TraceStep(const c8* text, u32 a, u32 b) { Trace(text, a, b); }

void ModelLoader::BackToIdle(LoadedModel* entry) {
#ifdef GAME_ORAS
  Trace("idle");
  entry->is_amie = false;
  entry->is_one_shot = false;
  ModelMotion::Play(entry->motion, 0);
  ModelMotion::SetLoop(entry->motion, true);
#else
  (void)entry;
#endif
}

bool ModelLoader::StartAction(LoadedModel* entry, u32 kind) {
#ifdef GAME_ORAS
  const bool use_amie = entry->amie_motion != nullptr &&
                        (kind == 1 || (kind == 2 && core::Utils::GetRandomValue(2) == 0));
  void* motion = use_amie ? entry->amie_motion : entry->motion;
  const s32* list = use_amie ? kAmieActions : kBattleActions;
  const u32 count = use_amie ? SIZE(kAmieActions) : SIZE(kBattleActions);
  // A model can miss some animations: try a few.
  for (u32 attempt = 0; attempt < 4; attempt++) {
    const s32 type = list[core::Utils::GetRandomValue(count)];
    Trace("try", use_amie ? 1 : 0, type);
    if (!ModelMotion::IsAvailable(motion, type)) continue;
    Trace("available");
    if (!ModelMotion::Play(motion, type)) continue;
    Trace("playing");
    ModelMotion::SetLoop(motion, false);
    entry->is_amie = use_amie;
    entry->is_one_shot = true;
    return true;
  }
#else
  (void)entry;
  (void)kind;
#endif
  return false;
}

void ModelLoader::Act(LoadedModel* entry, bool use_amie) {
#ifdef GAME_ORAS
  if (entry == nullptr || entry->motion == nullptr) return;
  Trace("act", use_amie ? 1 : 0);
  StartAction(entry, use_amie ? 1 : 0);
  entry->quiet_frames = 0;
  Trace("cry", static_cast<u32>(entry->species), static_cast<u32>(entry->form));
  ((void (*)(u32, u32, u32, u32, bool))address::kPlayCry)(
      0, static_cast<u32>(entry->species), static_cast<u32>(entry->form), 0,
      false);
  Trace("cry played");
  // The cry is quiet in the overworld: raise the volume of the voice 0.
  if (READ32(address::kVoiceManager + 12) > 0) {
    u8* voice = reinterpret_cast<u8*>(READ32(address::kVoicePlayers));
    if (voice != nullptr && (voice[24] == 2 || voice[24] == 4)) {
      *reinterpret_cast<f32*>(voice + 0x20) = kCryVolume;
      *reinterpret_cast<f32*>(voice + 0x34) = 0.0f;
      *reinterpret_cast<u32*>(voice + 0x1C) |= 1;
    }
  }
#else
  (void)entry;
  (void)use_amie;
#endif
}

void ModelLoader::UpdateMotions() {
#ifdef GAME_ORAS
  auto& context = GetInstance();
  static u32 frame = 0;
  UpdatePokemonLight();
  if (++frame % 60 == 0) {
    Trace("alive heap/device KB", core::GameFile::GetFreeHeapMemory(0) / 1024,
          core::GameFile::GetFreeHeapMemory(1) / 1024);
  }
  for (u32 i = 0; i < context.loaded_count_; ++i) {
    LoadedModel* entry = context.loaded_[i];
    if (entry == nullptr || entry->motion == nullptr || entry->model == nullptr) {
      continue;
    }
    // An animation of Pokémon ends, or stops when the Pokémon walks. After
    // some time without movement, the Pokémon plays a new one.
    if (entry->is_walking) {
      if (entry->is_one_shot) BackToIdle(entry);
      if (entry->quiet_frames < entry->next_action_frames) entry->quiet_frames++;
    } else if (entry->is_one_shot) {
      if (ModelMotion::IsFinished(ActiveMotion(*entry))) BackToIdle(entry);
    } else if (++entry->quiet_frames >= entry->next_action_frames) {
      StartAction(entry, 2);
      entry->quiet_frames = 0;
      entry->next_action_frames =
          kQuietFramesMin + core::Utils::GetRandomValue(kQuietFramesRandom);
    }

    void* active = ActiveMotion(*entry);
    entry->walk.Advance(entry->is_walking);
    ModelMotion::SetRoot(active, entry->model->position,
                         entry->model->rotation, entry->model->scale);
    ModelMotion::Advance(active);
  }
#endif
}

void ModelLoader::ApplyMotion(const void* model) {
#ifdef GAME_ORAS
  auto& context = GetInstance();
  for (u32 i = 0; i < context.loaded_count_; ++i) {
    LoadedModel* entry = context.loaded_[i];
    if (entry != nullptr && entry->motion != nullptr && entry->model == model) {
      ModelMotion::Apply(ActiveMotion(*entry), entry->model);
      entry->walk.Apply(reinterpret_cast<renderer::H3dModel*>(entry->model));
      return;
    }
  }
#else
  (void)model;
#endif
}

void ModelLoader::ApplyPokemonLight(renderer::H3dShaderModel* model,
                                    void* scene) {
#ifdef GAME_ORAS
  // The shaders of the Pokémon read a light set of their own. The game makes
  // one light for them, and a Pokémon of the overworld uses the set 15.
  // The light belongs to a scene: a new scene needs a new light.
  if (g_light_scene != scene) {
    ((void (*)(void*, void*, void**, void*, s32, s32))
        renderer::address::kCreateSceneLight)(HeapAllocator(),
                                              DeviceAllocator(),
                                              g_light_and_set, scene, 0,
                                              kPokemonLightSet);
    g_light_scene = scene;
  }
  auto* own = reinterpret_cast<renderer::H3dModel*>(model);
  const s32 material_count = own->GetMaterialCount();
  for (s32 mat = 0; mat < material_count; ++mat) {
    own->SetLightSet(mat, kPokemonLightSet);
  }
#else
  (void)model;
  (void)scene;
#endif
}

void ModelLoader::Track(LoadedModel* entry) {
  auto& context = GetInstance();
  if (context.loaded_count_ >= kMaxLoaded) return;
  context.loaded_[context.loaded_count_++] = entry;
}

void ModelLoader::ReportUnresolved(renderer::H3dResource* resource) {
  ui::Log::Print(u"setup failed, unresolved %X",
                            resource->GetState());
}

} // namespace pokemon
