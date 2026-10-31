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
 * @file item_extension.cc
 * @brief Adds new items to the game.
 *
 * The declarations are in battle/patch/item_extension.h.
 */

#include "battle/patch/item_extension.h"
#include "core/data_folder.h"

#include "battle/address.h"
#include "battle/constant/listener_source.h"
#include "battle/constant/priority_tier.h"
#include "battle/native/broadcaster.h"
#include "battle/native/listener.h"
#include "battle/native/pokemon.h"
#include "core/hook.h"
#include "core/memory.h"
#include "core/native/game_manager.h"
#include "core/native/process_manager.h"
#include "core/patch/archive.h"
#include "pokemon/address.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/evolution_data.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/runtime_data.h"
#include "savedata/native/pokemon_param.h"
#include "system/address.h"
#include "core/problems.h"

namespace battle {

namespace {
core::Hook<void(pokemon::ItemData*, ItemId, void*)> item_data_hook;
core::Hook<void(pokemon::ItemData*, String*, void*)> item_name_hook;
core::Hook<u32(void*, u32, u32, String*)> get_text_of_file_hook;
core::Hook<bool(void*, u16, u16, void*)> recover_check_hook;
core::Hook<bool(void*, u16, u16, u16, void*, void*)> recover_hook;
core::Hook<bool(void*, u32*, u32, u32, u32)> hp_bar_step_hook;
core::Hook<uptr(Pokemon*)> register_item_hook;
core::Hook<void(Pokemon*)> register_item_checked_hook;
core::Hook<uptr(Pokemon*, ItemId)> register_temporary_item_hook;

// The item that lends its data to a new item before patch_data runs: an item
// with no effect.
constexpr ItemId kTemplateItem = ItemId::kHeartScale;
// The number of the check of the second registration function.
constexpr u32 kCheckNumber = 5;

// true for an id of a new item (not an item of the game).
bool IsNewItem(ItemId id) {
  return static_cast<u32>(id) >= ItemExtension::kFirstId;
}

// true when the spec has reactions that replace the reactions of the game.
bool HasReactions(const ItemSpec* spec) {
  return spec != nullptr && spec->reactions != nullptr &&
         spec->reaction_count != 0;
}

// Registers the reactions of an item for a Pokémon that holds it.
uptr RegisterReactions(Pokemon* pkm, const ItemSpec* spec) {
  if (spec->reactions == nullptr || spec->reaction_count == 0) return 0;
  return (uptr)Broadcaster::Register(
      ListenerSource::kHeldItem, static_cast<u32>(spec->id),
      PriorityTier::kHeldItemDefault, pkm->speed, UID{pkm->uid},
      const_cast<ReactionTable*>(spec->reactions), spec->reaction_count);
}
} // namespace

void ItemExtension::Initialize() {
#ifdef GAME_ORAS
  item_data_hook.Install(pokemon::address::kItemDataInitialize, ItemDataHook);
  item_name_hook.Install(pokemon::address::kItemDataGetName, ItemNameHook);
  get_text_of_file_hook.Install(sys::address::kGetTextOfFile,
                                GetTextOfFileHook);
  // The Bag calls these two functions of the game to use a medicine.
  recover_check_hook.Install(pokemon::address::kItemRecoverCheck,
                             RecoverCheckHook);
  recover_hook.Install(pokemon::address::kItemRecover, RecoverHook);
  // The battle code is loaded only during a battle: the plugin enables these
  // hooks when a battle starts.
  register_item_hook.Install(address::kRegisterItemListener, RegisterItemHook,
                             address::kVtable);
  register_item_checked_hook.Install(address::kRegisterItemListenerChecked,
                                     RegisterItemCheckedHook, address::kVtable);
  register_temporary_item_hook.Install(address::kRegisterTemporaryItemListener,
                                       RegisterTemporaryItemHook,
                                       address::kVtable);
#endif
}

bool ItemExtension::AddItem(const ItemSpec& spec) {
  auto& self = GetInstance();
  const u32 id = static_cast<u32>(spec.id);
  const bool is_game_item = id >= 1 && id < kGameItemCount;
  const bool is_new_item = id >= kFirstId && id <= kLastId;
  if (self.item_count_ >= kMaxItems) {
    core::Problems::Add(u"item %u: more than %u items", id, kMaxItems);
    return false;
  }
  if (!is_game_item && !is_new_item) {
    core::Problems::Add(u"item %u: use an id from %u to %u", id, kFirstId,
                        kLastId);
    return false;
  }
  if (FindItem(spec.id) != nullptr) {
    core::Problems::Add(u"item %u: the id is used two times", id);
    return false;
  }
  self.items_[self.item_count_++] = spec;
  MakeIconTable();
  if (!self.is_icon_table_ready_) return true;

  // The icon of its own, else the icon of an item of the game.
  c16 path[96];
  const u32 icon_file = kFirstIconFile + self.icon_file_count_;
  const u32 icon_item = static_cast<u32>(spec.icon_item);
  if (GetIconPath(spec, path, SIZE(path)) &&
      core::Archive::RegisterItemIcon(icon_file, path)) {
    self.icon_table_[id] = icon_file;
    self.icon_file_count_++;
  } else if (icon_item != 0 && icon_item < kGameItemCount) {
    self.icon_table_[id] = self.icon_table_[icon_item];
  }
  return true;
}

bool ItemExtension::GetIconPath(const ItemSpec& spec, c16* path,
                                u32 capacity) {
  static const c16 kFolder[] = SANGO_DATA_PATH(u"items/");
  static const c16 kExtension[] = u".bclim";
  if (spec.name == nullptr) return false;
  u32 length = 0;
  for (const c16* c = kFolder; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  for (const c16* c = spec.name; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  for (const c16* c = kExtension; *c != 0; c++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *c;
  }
  path[length] = 0;
  return true;
}

const ItemSpec* ItemExtension::FindItem(ItemId id) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.item_count_; i++) {
    if (self.items_[i].id == id) return &self.items_[i];
  }
  return nullptr;
}

u32 ItemExtension::GetIconFile(ItemId id) {
  auto& self = GetInstance();
  const u32 index = static_cast<u32>(id);
  if (!self.is_icon_table_ready_ || FindItem(id) == nullptr ||
      index >= kIconTableSize) {
    return 0xFFFFFFFF;
  }
  return self.icon_table_[index];
}

bool ItemExtension::PatchName(ItemId item, String* output) {
  const ItemSpec* spec = FindItem(item);
  if (spec == nullptr || spec->name == nullptr) return false;
  output->Set(spec->name);
  return true;
}

bool ItemExtension::PatchDescription(ItemId item, String* output) {
  const ItemSpec* spec = FindItem(item);
  if (spec == nullptr || spec->description == nullptr) return false;
  output->Set(spec->description);
  return true;
}

// The game reads the text of an item (name, description) with this function:
// the plugin keeps the number of the text file while the game reads.
u32 ItemExtension::GetTextOfFileHook(void* context, u32 text_id, u32 file_id,
                                     String* output) {
  auto& self = GetInstance();
  const u32 previous = self.current_text_file_;
  self.current_text_file_ = file_id;
  const u32 result = get_text_of_file_hook(context, text_id, file_id, output);
  self.current_text_file_ = previous;
  return result;
}

u32 ItemExtension::ResolveTextFile(u32 file_id) {
  return file_id != 0 ? file_id : GetInstance().current_text_file_;
}

// The battle code compares the item of a Pokémon with 776 (the number of items
// of the game) and removes an item that is above it. The patch changes the
// number to 1024 (CMP R0, #0x400).
void ItemExtension::PatchBattleLoad() {
#ifdef GAME_ORAS
  constexpr u32 kCompareWith1024 = 0xE3500B01;
  WRITE32(address::kItemLimitCheck1, kCompareWith1024);
  WRITE32(address::kItemLimitCheck2, kCompareWith1024);
#endif
}

// The Bag reads the data of an item from the archive of the item data, and not
// with the function of the plugin. The plugin builds the data of each new item
// one time (the game does it with the hooks above), and the archive gives it to
// the Bag.
void ItemExtension::Update() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  if (self.is_data_given_ || self.item_count_ == 0) return;
  if (core::GameManager::GetInstance().GetSystemHeap() == nullptr ||
      !core::ProcessManager::IsOverworldActive()) {
    return;
  }
  for (u32 i = 0; i < self.item_count_; i++) {
    const ItemId id = self.items_[i].id;
    if (!IsNewItem(id)) continue;
    pokemon::ItemData data(id);
    core::Archive::RegisterItemData(static_cast<u32>(id),
                                    reinterpret_cast<const u8*>(&data));
  }
  self.is_data_given_ = true;
#endif
}

// The game finds the file of an item icon in a table of 776 numbers, in three
// places of its code. The plugin makes a bigger table (a copy of the table of
// the game, then the new items) and writes its address in the three places.
void ItemExtension::MakeIconTable() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  if (self.is_icon_table_ready_) return;
  for (u32 i = 0; i < kGameItemCount; i++) {
    self.icon_table_[i] = READ32(pokemon::address::kItemIconTable + i * 4);
  }
  const uptr users[] = {pokemon::address::kItemIconTableUser1,
                        pokemon::address::kItemIconTableUser2,
                        pokemon::address::kItemIconTableUser3};
  for (uptr user : users) {
    WRITE32(user, reinterpret_cast<u32>(self.icon_table_));
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, user, 4);
  }
  self.is_icon_table_ready_ = true;
#endif
}

// Adds the evolution of each evolution stone to the evolutions that the game
// just loaded for a species: "use this item" gives the species of the item. The
// game then does the check, the animation of the evolution and the change.
void ItemExtension::PatchEvolution(SpeciesId species,
                                   pokemon::EvolutionData& table) {
  auto& self = GetInstance();
  for (u32 i = 0; i < self.item_count_; i++) {
    const ItemSpec& spec = self.items_[i];
    if (spec.evolve_into == nullptr) continue;
    const SpeciesId target = spec.evolve_into(species, table);
    if (target == SpeciesId::kNone) continue;

    // The first free place of the table.
    for (auto& entry : table.data) {
      if (entry.method != pokemon::EvolutionMethod::kNone) continue;
      entry.method = pokemon::EvolutionMethod::kItem;
      entry.item = spec.id;
      entry.species = target;
      break;
    }
  }
}

// The Bag treats an item as a medicine for a Pokémon when its data says so: a
// function of the overworld (1: medicine), "use on a Pokémon", and no battle
// function. The flag "restores all the PP" makes the Bag show one fixed message,
// which the plugin replaces (see PatchUseMessage()).
void ItemExtension::MakeUsableInBag(pokemon::ItemData& item,
                                    const ItemSpec& spec) {
  constexpr u8 kMedicineFunction = 1;
  item.overworld_function = kMedicineFunction;
  item.battle_function = 0;
  item.use_on_pokemon = true;
  item.use_no_consume = 0;
  item.cure_sleep = item.cure_poison = item.cure_burn = item.cure_freeze = 0;
  item.cure_paralysis = item.cure_confusion = item.cure_infatuation = 0;
  item.revive = item.revive_all = item.level_up = item.evolve = 0;
  item.restore_pp = 0;
  item.restore_hp = 0;
  // An evolution stone has the flag "evolution": the Bag starts the evolution
  // of the game. The other items use the message of the PP.
  item.evolve = spec.evolve_into != nullptr ? 1 : 0;
  item.restore_all_pp = spec.use != nullptr ? 1 : 0;
  // An item that changes the HP has the type "HP": the Bag plays the animation
  // of the HP bar and shows the message of the HP.
  if (spec.changes_hp) {
    item.restore_hp = 1;
    item.restore_all_pp = 0;
  }
}

// The Bag code is a CRO: it is in memory only while the Bag runs. The first
// instructions of the function must be the ones that the plugin knows, or the
// jump of the hook, else the plugin does not touch the memory.
void ItemExtension::EnableHpBarHook() {
  constexpr u32 kFirst = 0xE591C000; // ldr ip, [r1]
  constexpr u32 kSecond = 0xE15C0002; // cmp ip, r2
  constexpr u32 kJump = 0xE51FF004; // the jump of a hook
  const u32 address = pokemon::address::kBagHpBarStep;
  if (address == 0) return;
  MemInfo info;
  PageInfo page;
  if (R_FAILED(svcQueryMemory(&info, &page, address))) return;
  if ((info.perm & MEMPERM_READ) == 0) return;
  const u32 first = READ32(address);
  if (first != kFirst && first != kJump) return;
  if (first == kFirst && READ32(address + 4) != kSecond) return;
  hp_bar_step_hook.Install(address, HpBarStepHook, /*process=*/1);
  hp_bar_step_hook.Enable(true);
}

// The Bag makes the HP bar count up, one HP for each frame, until the new HP.
// When the new HP are lower, the bar counts down in the same way. The function
// of the game only counts up: to count down, it runs for a counter that is two
// below the target, and a new HP that is one below, so it makes one step.
bool ItemExtension::HpBarStepHook(void* bag, u32* counter, u32 hp, u32 max_hp,
                                  u32 place) {
  if (*counter <= hp) return hp_bar_step_hook(bag, counter, hp, max_hp, place);
  if (*counter < 2) {
    *counter = hp;
    return true;
  }
  u32 step = *counter - 2;
  hp_bar_step_hook(bag, &step, *counter - 1, max_hp, place);
  *counter = step;
  return false;
}

// The game reads the data of an item into a work buffer. For a new item, the
// game loads the template item, then patch_data edits the copy and the id is
// set back.
void ItemExtension::ItemDataHook(pokemon::ItemData* item, ItemId id,
                                 void* heap) {
  const ItemSpec* spec = FindItem(id);
  // A new item starts as the template item. An item of the game keeps its data.
  const bool uses_template = spec != nullptr && IsNewItem(id);
  item_data_hook(item, uses_template ? kTemplateItem : id, heap);
  if (spec == nullptr) return;
  if (spec->use != nullptr || spec->evolve_into != nullptr) {
    MakeUsableInBag(*item, *spec);
  }
  if (spec->patch_data != nullptr) spec->patch_data(*item);
  item->id = id;
}

namespace {
// Runs a function of the plugin on the data of a Pokémon: the data is encrypted
// in the game, so it is decrypted first and encrypted again after.
template <typename F>
void WithCoreData(void* pokemon, F function) {
  auto* param = static_cast<savedata::PokemonParam*>(pokemon);
  param->accessor->Decrypt();
  function(*param->core);
  param->accessor->Encrypt();
}

// Writes `text` in `out`, and replaces `{pokemon}` with the name of the Pokémon.
void ExpandMessage(const c16* text, const pokemon::CoreData& core, c16* out,
                   u32 capacity) {
  static const c16 kTag[] = u"{pokemon}";
  u32 length = 0;
  for (const c16* c = text; *c != 0 && length + 1 < capacity;) {
    bool is_tag = true;
    for (u32 i = 0; kTag[i] != 0; i++) {
      if (c[i] != kTag[i]) {
        is_tag = false;
        break;
      }
    }
    if (is_tag) {
      for (u32 i = 0; core.nickname[i] != 0 && i < 12 && length + 1 < capacity;
           i++) {
        out[length++] = core.nickname[i];
      }
      c += 9;
    } else {
      out[length++] = *c++;
    }
  }
  out[length] = 0;
}
} // namespace

// The Bag asks if a medicine can act on a Pokémon. A new usable item asks its
// own function.
bool ItemExtension::RecoverCheckHook(void* pokemon, u16 item, u16 move,
                                     void* heap) {
  auto& self = GetInstance();
  self.has_use_message_ = false;
  const ItemSpec* spec = FindItem(static_cast<ItemId>(item));
  if (spec == nullptr || spec->use == nullptr) {
    return recover_check_hook(pokemon, item, move, heap);
  }
  if (spec->changes_hp) EnableHpBarHook();
  if (spec->can_use == nullptr) return true;
  bool can_use = false;
  WithCoreData(pokemon, [&](pokemon::CoreData& core) {
    can_use = spec->can_use(core);
  });
  return can_use;
}

// The Bag uses a medicine on a Pokémon. A new usable item runs its function and
// keeps its message for the Bag.
bool ItemExtension::RecoverHook(void* pokemon, u16 item, u16 move, u16 place,
                                void* helper, void* heap) {
  auto& self = GetInstance();
  const ItemSpec* spec = FindItem(static_cast<ItemId>(item));
  if (spec == nullptr || spec->use == nullptr) {
    self.has_use_message_ = false;
    return recover_hook(pokemon, item, move, place, helper, heap);
  }
  self.hp_delta_ = 0;
  WithCoreData(pokemon, [&](pokemon::CoreData& core) {
    const c16* message = spec->use(core);
    static const c16 kDefault[] = u"{pokemon} used the item.";
    ExpandMessage(message != nullptr ? message : kDefault, core,
                  self.use_message_, SIZE(self.use_message_));
  });
  self.has_use_message_ = true;
  auto* param = static_cast<savedata::PokemonParam*>(pokemon);
  param->UpdateRuntimeData();
  if (self.hp_delta_ != 0) {
    // The HP are in the calculated data, which is encrypted like the rest: change
    // them after the calculation, between a Decrypt and an Encrypt.
    param->accessor->Decrypt();
    s32 hp = param->runtime->hp + self.hp_delta_;
    if (hp > param->runtime->max_hp) hp = param->runtime->max_hp;
    if (hp < 1) hp = 1;
    param->runtime->hp = static_cast<u16>(hp);
    param->accessor->Encrypt();
    self.hp_delta_ = 0;
  }
  return true;
}

bool ItemExtension::PatchUseMessage(u32 file_id, u32 text_id, String* output) {
  // The text file of the Bag, and the text "The PP were restored".
  constexpr u32 kBagFile = 164;
  constexpr u32 kPpRestoredText = 114;
  // The messages of the HP: "recovered X HP", and "no HP recovered".
  constexpr u32 kHpRecoveredText = 109;
  constexpr u32 kHpNotRecoveredText = 116;
  auto& self = GetInstance();
  if (!self.has_use_message_ || file_id != kBagFile ||
      (text_id != kPpRestoredText && text_id != kHpRecoveredText &&
       text_id != kHpNotRecoveredText)) {
    return false;
  }
  output->Set(self.use_message_);
  return true;
}

// The game writes the name of an item from the data of the item.
void ItemExtension::ItemNameHook(pokemon::ItemData* item, String* output,
                                 void* heap) {
  if (PatchName(item->id, output)) return;
  item_name_hook(item, output, heap);
}

uptr ItemExtension::RegisterItemHook(Pokemon* pkm) {
  const ItemSpec* spec = FindItem(pkm->item);
  if (!HasReactions(spec)) return register_item_hook(pkm);
  return RegisterReactions(pkm, spec);
}

void ItemExtension::RegisterItemCheckedHook(Pokemon* pkm) {
  const ItemSpec* spec = FindItem(pkm->item);
  if (!HasReactions(spec)) {
    register_item_checked_hook(pkm);
    return;
  }
  // The check of the game: it can refuse the registration.
  const bool is_refused =
      ((u32(*)(u32, u32))address::kItemListenerCheck)(pkm->uid, kCheckNumber) !=
      0;
  if (!is_refused) RegisterReactions(pkm, spec);
}

uptr ItemExtension::RegisterTemporaryItemHook(Pokemon* pkm, ItemId item) {
  const ItemSpec* spec = FindItem(item);
  if (!HasReactions(spec)) return register_temporary_item_hook(pkm, item);
  return RegisterReactions(pkm, spec);
}

} // namespace battle
