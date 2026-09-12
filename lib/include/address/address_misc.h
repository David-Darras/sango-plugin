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

/** * @name Miscellaneous Addresses
 * @{ */
#define ADDRESS_CORE (0x0063106C)
#define ADDRESS_UPDATE_FRAME (0x0011EEA4)
#define ADDRESS_EVENT_TABLE_CHECK_FLAG (0x0012FE90)
#define ADDRESS_EVENT_TABLE_RESET_FLAG (0x0011FB38)
#define ADDRESS_EVENT_TABLE_SET_FLAG (0x00406E94)
#define ADDRESS_ALLOC (0x001235A8)
#define ADDRESS_DO_FIELD_MOVE (0x003D40EC)
#define ADDRESS_RECORD_MAX_VALUE_TABLE (0x0058DE14) // 999999999, 9999999, etc.
#define ADDRESS_RECORD_MAX_VALUE_INDEX_TABLE (0x0058DD4A) // 0, 0, 0, 1, 1, etc.
#define ADDRESS_IS_SHINY (0x00168F48)
#define ADDRESS_TRAINER_MODEL_TABLE (0x00586B8A)
#define ADDRESS_CHECK_REGULATION (0x006F4F80) // app poke list static work
// 0x8072520, 0x807251C, 0x8072510
#define ADDRESS_LOAD_CRO_FILE (0x00110E2C)
#define ADDRESS_START_BACKUP_THREAD (0x0045D6BC)
#define ADDRESS_ON_UPDATE_FRAME (0x0011EEA4)
#define ADDRESS_KEYBOARD_UPDATE_KEYS (0x00744540)
/** @} */

/** * @name Field Script (Pawn) Addresses
 * The script engine (script::Engine) drives every field script;
 * gfl::pawn::PawnBase wraps the AMX virtual machine underneath it.
 * @{ */
/// script::Engine* — the engine singleton, null outside of the field
#define ADDRESS_SCRIPT_ENGINE_INSTANCE (0x005F46FC)
/// Fills a script::ScriptDescriptor (this, zone_id, script_id, zone_data)
#define ADDRESS_SCRIPT_DESCRIPTOR_SETUP (0x003FB594)
/// gfl::pawn::PawnBase::Load(buffer, size, amx_name)
#define ADDRESS_SCRIPT_PAWN_BASE_LOAD (0x003AB0C8)
/// amx_Init(amx, program)
#define ADDRESS_SCRIPT_INITIALIZE_VIRTUAL_MACHINE (0x00505DB4)
/// amx_RaiseError(amx, error)
#define ADDRESS_SCRIPT_RAISE_ERROR (0x0050630C)
/// Native tables: {name, function} pairs, {0, 0} ends each one. Named after
/// what they contain.
/// keys, sound, text, talking, movement, camera, battle and app calls
#define ADDRESS_SCRIPT_NATIVES_FIELD (0x007EA58C)
/// flags, variables, NPC placement, items, time, party, dex, player, records
#define ADDRESS_SCRIPT_NATIVES_STATE (0x007EB234)
/// interactive NPC events
#define ADDRESS_SCRIPT_NATIVES_INTERACTIVE (0x007EB99C)
/// Pokémon Center: healing machine balls, icons, trainer card
#define ADDRESS_SCRIPT_NATIVES_POKEMON_CENTER (0x007EB9FC)
/// gym puzzles, pop-up messages, League and Hall of Fame effects
#define ADDRESS_SCRIPT_NATIVES_MAP_EFFECTS (0x007EBAAC)
/// Battle Maison / battle facilities
#define ADDRESS_SCRIPT_NATIVES_BATTLE_FACILITY (0x007EBB44)
/// saving, photos, day care, secret bases, berries, trainer eyes, races
#define ADDRESS_SCRIPT_NATIVES_FIELD_SERVICES (0x007EBCDC)
/// NPC AI
#define ADDRESS_SCRIPT_NATIVES_NPC_AI (0x007EBFBC)
/** @} */

/** * @name Shop Addresses
 * app::shop::ItemContainer holds what a counter sells, field::EventShopBuyCall
 * drives the counter itself.
 * @{ */
#define ADDRESS_LOAD_SHOP_ITEMS (0x00346554)
#define ADDRESS_UNLOAD_SHOP_ITEMS (0x00346AE0)
#define ADDRESS_SHOP_GET_ITEM_NAME (0x00346A44)
#define ADDRESS_SHOP_GET_ITEM_INFO (0x003469E0)
#define ADDRESS_SHOP_DISPLAY_ITEM_INFO (0x003E6550)
#define ADDRESS_SHOP_PURCHASE_ITEM (0x003E6764)
#define ADDRESS_BAG_ADD_ITEM (0x00463750)
/** @} */

/** * @name In-Game Application (Menu) Addresses
 * @{ */
/// Allocation size immediates patched so the hooked app gets 0x100 extra bytes.
#define ADDRESS_OVERWORLD_MENU_APP_ALLOC_SIZE (0x003D6D18)
#define ADDRESS_TOP_MENU_APP_ALLOC_SIZE (0x003D81D4)
#define ADDRESS_TOWN_MAP_CALLBACK (0x003EF898)
/** @} */
