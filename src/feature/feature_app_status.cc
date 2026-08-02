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

#include "feature/feature_app_status.h"

#include "game/process_manager.h"
#include "game/renderer/app_layout_manager.h"
#include "game/constant/ability.h"
#include "game/constant/form.h"
#include "game/constant/species.h"

#define NEXT(x, val, max) x = ((x) + (val)) % (max)
#define PREV(x, val, max) x = ((x) - (val) + (max)) % (max)

#define ENUM_NEXT(type, x, val) x = static_cast<type>(( static_cast<u32>(x) + 1) % static_cast<u32>(type::kMax))
#define ENUM_PREV(type, x, val) x = static_cast<type>(( static_cast<u32>(x) - 1 + static_cast<u32>(type::kMax)) % static_cast<u32>(type::kMax))

/*
 * TODO:
 * Print current mode
 *
 * [Idle]
 *   - up/down : update pokemon_index
 *   - left/right : update page_index
 *   - Y : open (use navigation mode)
 * [Navigation]
 *   - DPAD : Move across panes
 *   - L/R : switch subpages (iv/ev/stat)
 *   - A : select pane
 *   - B/Y : leave (use idle mode)
 * [Editing]
 *   - LEFT/RIGHT : decrease/increase value by one
 *   - DOWN/UP : decrease/increase value by ten
 *   - B : leave (use navigation mode)
 *
 * Hide : "/" (2), "<max hp>" (6)
 *
 * [PAGE 0]
 *   [GROUP 0] up-left
 *     - Dex No. <value> (58/60)
 *     - ID No. <value> (62/64)
 *     - To Next Lv. -> Shiny <no/yes> (34/36)
 *   [GROUP 1]
 *     [SUBPAGE 0] up-right
 *       - HP <value> (0/4)
 *       - Attack <value> (8/10)
 *       - Defense <value> (12/14)
 *       - Sp. Atk <value> (16/18)
 *       - Sp. Def <value> (20/22)
 *       - Speed <value> (24/26)
 *     [SUBPAGE 1]
 *       - IV HP <value>
 *       - IV Attack <value>
 *       - IV Defense <value>
 *       - IV Sp. Atk <value>
 *       - IV Sp. Def <value>
 *       - IV Speed <value>
 *     [SUBPAGE 2]
 *       - EV HP <value>
 *       - EV Attack <value>
 *       - EV Defense <value>
 *       - EV Sp. Atk <value>
 *       - EV Sp. Def <value>
 *       - EV Speed <value>
 *     - Ability + Ability info (38/40)
 *   [GROUP 2] down-left
 *     - Lv. <value>
 *     - <item>
 *   [GROUP 3] down-right
 *     - <move 0>
 *     - <pp move 0>
 *     - <move 1>
 *     - <pp move 1>
 *     - <move 2>
 *     - <pp move 2>
 *     - <move 3>
 *     - <pp move 3>
 */

namespace game {
class BaseProcess;
}

namespace feature {
class Pane {
public:
  Pane(u8 label_id, void (*prev)(PokemonCoreData& core, u32 value),
       void (*next)(PokemonCoreData& core, u32 value),
       void (*print)(Pane* pane, AppLayoutManager& manager,
                     PokemonDataAccessor& accessor) =
           nullptr) :
    label_pane_id_(label_id),
    value_pane_id_(label_id + 2),
    prev_(prev),
    next_(next), print_(print) {
  }

  Pane(u8 label_id, u8 value_id, u8 text_id,
       void (*prev)(PokemonCoreData& core, u32 value),
       void (*next)(PokemonCoreData& core, u32 value),
       void (*print)(Pane* pane, AppLayoutManager& manager,
                     PokemonDataAccessor& accessor) =
           nullptr) :
    Pane(label_id, prev, next, print) {
    value_pane_id_ = value_id;
    text_id_ = text_id;
  }

  void Draw(AppLayoutManager& manager, PokemonDataAccessor& accessor,
            Color8& top,
            Color8& bottom) {
    SetColors(manager, top, bottom);
    if (print_)
      print_(this, manager, accessor);
  }

  void SetColors(AppLayoutManager& manager, Color8& top, Color8& bottom) {
    manager.SetTextBoxColor(0, label_pane_id_, top, bottom);
    if (value_pane_id_ != 0xFF)
      manager.SetTextBoxColor(0, value_pane_id_, top, bottom);
  }

  void Prev(PokemonCoreData& core, u32 value) { if (prev_) prev_(core, value); }
  void Next(PokemonCoreData& core, u32 value) { if (next_) next_(core, value); }

public:
  void (*prev_)(PokemonCoreData& core, u32 value);
  void (*next_)(PokemonCoreData& core, u32 value);
  void (*print_)(Pane* pane, AppLayoutManager& manager,
                 PokemonDataAccessor& accessor);
  u8 label_pane_id_ = 0;
  u8 value_pane_id_ = 0;
  u8 text_id_ = 0;
};

#define DEFINE_PANE(id, name, field, max)\
static void next_##name(PokemonCoreData& core, u32 value) {\
NEXT(core.field, value, max);\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
PREV(core.field, value, max);\
}\
static Pane pane_##name(id, prev_##name, next_##name);

DEFINE_PANE(58, dex_no, species, SPECIES_COUNT)
DEFINE_PANE(62, id_no, id, 0xFFFFFFFF)
DEFINE_PANE(38, ability, ability, ABILITY_COUNT)

#undef DEFINE_PANE

#define STAT_PANE_FUNCS(str, name, id0, id1, id2, stat_field, iv_field, ev_field)\
static void next_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.page_ == AppStatus::Page::kIv) {\
NEXT(core.iv_field, value, 31 + 1);\
} else if (ctx.page_ == AppStatus::Page::kEv) {\
NEXT(core.ev_field, value, 252 + 1);\
}\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.page_ == AppStatus::Page::kIv) {\
PREV(core.iv_field, value, 31 + 1);\
} else if (ctx.page_ == AppStatus::Page::kEv) {\
PREV(core.ev_field, value, 252 + 1);\
}\
}\
static void print_##name(Pane* pane, AppLayoutManager& manager, PokemonDataAccessor& accessor) {\
auto& ctx = feature::AppStatus::GetInstance();\
auto& core = *accessor.GetCoreData();\
auto& runtime = *accessor.GetRuntimeData();\
if (ctx.page_ == AppStatus::Page::kIv) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"IV %s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)core.iv_field, 2);\
} else if (ctx.page_ == AppStatus::Page::kEv) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"EV %s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)core.ev_field, 3);\
} else if (ctx.page_ == AppStatus::Page::kStat) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"%s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)runtime.stat_field, 3);\
}\
}\
static Pane pane_##name(id0, id1, id2, prev_##name, next_##name, print_##name);

STAT_PANE_FUNCS("HP", hp, 0, 4, 17, hp, iv_hp, ev_hp)
STAT_PANE_FUNCS("Attack", attack, 8, 10, 19, attack, iv_attack, ev_attack)
STAT_PANE_FUNCS("Defense", defense, 12, 14, 21, defense, iv_defense, ev_defense)
STAT_PANE_FUNCS("Sp. Atk", sp_attack, 16, 18, 23, special_attack,
                iv_special_attack,
                ev_special_attack)
STAT_PANE_FUNCS("Sp. Def", sp_defense, 20, 22, 25, special_defense,
                iv_special_defense,
                ev_special_defense)
STAT_PANE_FUNCS("Speed", speed, 24, 26, 27, speed, iv_speed, ev_speed)

#undef STAT_PANE_FUNCS

static void next_form_no(PokemonCoreData& core, u32 value) {
  NEXT(core.form, value, FORM_UNOWN_COUNT);
}

static void prev_form_no(PokemonCoreData& core, u32 value) {
  PREV(core.form, value, FORM_UNOWN_COUNT);
}

static void print_form_no(Pane* pane, AppLayoutManager& manager,
                          PokemonDataAccessor& accessor) {
  manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"Form No.");
  manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_,
                                 accessor.GetCoreData()->form, 2);
}

static Pane pane_form_no(34, 36, 14, prev_form_no, next_form_no, print_form_no);

static void toggle_shiny(PokemonCoreData& core, u32 value) {
  if (PokemonUtils::IsShiny(core.id, core.shiny_id)) {
    PokemonUtils::ConvertToNormal(core.id, &core.shiny_id);
  } else {
    PokemonUtils::ConvertToShiny(core.id, &core.shiny_id);
  }
}

static void print_shiny(Pane* pane, AppLayoutManager& manager,
                        PokemonDataAccessor& accessor) {
  manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"Is Shiny");
  auto& core = *accessor.GetCoreData();
  u32 is_shiny = PokemonUtils::IsShiny(core.id, core.shiny_id);
  manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_,
                                 is_shiny, 1);
}

static Pane pane_shiny(66, 68, 11, toggle_shiny, toggle_shiny, print_shiny);

struct Direction {
  Pane* center;
  Pane* up;
  Pane* right;
  Pane* down;
  Pane* left;
};

Direction DIRECTIONS[] = {
    // top-left
    {&pane_dex_no, nullptr, &pane_hp, &pane_id_no, nullptr},
    {&pane_id_no, &pane_dex_no, &pane_hp, &pane_shiny, nullptr},
    {&pane_shiny, &pane_id_no, &pane_hp, &pane_form_no, nullptr},
    {&pane_form_no, &pane_shiny, &pane_hp, nullptr, nullptr},
    // top-right
    {&pane_hp, nullptr, nullptr, &pane_attack, &pane_dex_no},
    {&pane_attack, &pane_hp, nullptr, &pane_defense, &pane_dex_no},
    {&pane_defense, &pane_attack, nullptr, &pane_sp_attack, &pane_dex_no},
    {&pane_sp_attack, &pane_defense, nullptr, &pane_sp_defense, &pane_dex_no},
    {&pane_sp_defense, &pane_sp_attack, nullptr, &pane_speed, &pane_dex_no},
    {&pane_speed, &pane_sp_defense, nullptr, &pane_ability, &pane_dex_no},
    {&pane_ability, &pane_speed, nullptr, nullptr, &pane_dex_no},
};

void AppStatus::Update(savedata::PokemonParam& pokemon,
                       Controller& controller) {
  const Key key = controller.GetRepeatedKey();
  const u8 pokemon_count = savedata::PokemonTeam::GetInstance().count;
  bool refresh = false;
  auto& accessor = *pokemon.accessor;

  switch (mode_) {
    case Mode::kIdle: {
      switch (key) {
        case Key::kDown:
          NEXT(slot_, 1, pokemon_count);
          break;
        case Key::kUp:
          PREV(slot_, 1, pokemon_count);
          break;
        case Key::kY:
          refresh = true;
          ChangeMode(Mode::kNavigation);
          break;
        default:
          break;
      }
      break;
    }

    case Mode::kNavigation: {
      refresh = true;
      switch (key) {
        case Key::kRight:
        case Key::kLeft:
        case Key::kUp:
        case Key::kDown:
          MoveAcrossPanes(controller);
          break;
        case Key::kR:
          ENUM_NEXT(Page, page_, 1);
          break;
        case Key::kL:
          ENUM_PREV(Page, page_, 1);
          break;
        case Key::kA:
          ChangeMode(Mode::kEditing);
          break;
        case Key::kY:
          ChangeMode(Mode::kIdle);
          break;
        default:
          break;
      }
      break;
    }

    case Mode::kEditing: {
      refresh = true;
      accessor.Decrypt();
      PokemonCoreData& core = *accessor.GetCoreData();
      switch (key) {
        case Key::kRight:
          current_pane_->Next(core, 1);
          break;
        case Key::kLeft:
          current_pane_->Prev(core, 1);
          break;
        case Key::kUp:
          current_pane_->Next(core, 10);
          break;
        case Key::kDown:
          current_pane_->Prev(core, 10);
          break;
        case Key::kR:
          current_pane_->Next(core, 100);
          break;
        case Key::kL:
          current_pane_->Prev(core, 100);
          break;
        case Key::kY:
        case Key::kA:
          ChangeMode(Mode::kNavigation);
          break;
        default:
          break;
      }
      accessor.Encrypt();
      pokemon.UpdateRuntimeData();
      break;
    }
  }

  if (refresh && key != Key::kNone) {
    bool new_model = mode_ == Mode::kEditing &&
                     (current_pane_ == &pane_dex_no || current_pane_ == &
                      pane_form_no ||
                      current_pane_ == &pane_shiny);
    game::BaseProcess* process = game::ProcessManager::GetInstance().
        GetCurrentProcess();
    bool& update_pokemon = *(bool*)(*(uptr*)((uptr)process + 104) + 284);
    if (!new_model) update_pokemon = false;
    ((void(*)(void*, u32, bool))ADDRESS_APP_STATUS_UPDATE_POKEMON)(process,
      slot_, false);
    if (!new_model) update_pokemon = true;
  }
}

void AppStatus::MoveAcrossPanes(Controller& controller) {
  Direction* direction = nullptr;
  for (u32 i = 0; i < SIZE(DIRECTIONS); i++) {
    if (DIRECTIONS[i].center == current_pane_) {
      direction = &DIRECTIONS[i];
      break;
    }
  }

  if (direction == nullptr) return;

  const Key key = controller.GetRepeatedKey();

  switch (key) {
    case Key::kUp:
      if (direction->up != nullptr) {
        current_pane_ = direction->up;
      }
      break;
    case Key::kRight:
      if (direction->right != nullptr) {
        current_pane_ = direction->right;
      }
      break;
    case Key::kDown:
      if (direction->down != nullptr) {
        current_pane_ = direction->down;
      }
      break;
    case Key::kLeft:
      if (direction->left != nullptr) {
        current_pane_ = direction->left;
      }
      break;
    default:
      break;
  }
}

void AppStatus::ChangeMode(Mode mode) {
  mode_ = mode;
  is_game_input_disabled_ = mode_ != Mode::kIdle;
}

void AppStatus::Draw(PokemonDataAccessor& accessor,
                     AppLayoutManager& manager) const {
  Color8 white(255, 255, 255, 255);
  accessor.Decrypt();
  for (u32 i = 0; i < SIZE(DIRECTIONS); i++) {
    Pane* pane = DIRECTIONS[i].center;
    pane->Draw(manager, accessor, white, white);
  }
  accessor.Encrypt();

  if (mode_ == Mode::kNavigation) {
    Color8 top(255, 0, 0, 255);
    Color8 bottom(255, 255, 0, 255);
    current_pane_->SetColors(manager, top, bottom);
  } else if (mode_ == Mode::kEditing) {
    Color8 top(255, 0, 255, 255);
    Color8 bottom(255, 255, 255, 255);
    current_pane_->SetColors(manager, top, bottom);
  } else {
    current_pane_->SetColors(manager, white, white);
  }
}

void AppStatus::SetupGraphicsHook(uptr self,
                                  savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphics, self, pokemon);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ(uptr, self + 8 + 16));

  ctx.Draw(*pokemon->accessor, manager);

  manager.Hide(0, 2); // don't show /
  manager.Hide(0, 6); // don't show max hp

  // pokemon->accessor->Decrypt();

  // pokemon->accessor->Encrypt();
}

void AppStatus::PatchOnUpdate() {
  auto& ctx = GetInstance();
  auto& team = savedata::PokemonTeam::GetInstance();
  auto& pokemon = *team.pokemons[ctx.slot_];
  auto& controller = Controller::GetInstance();

  ctx.Update(pokemon, controller);
}

void AppStatus::PatchOnLoad() {
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphics);
  auto& ctx = GetInstance();
  ctx.page_ = Page::kStat;
  ctx.slot_ = 0;
  ctx.ChangeMode(Mode::kIdle);
  ctx.current_pane_ = DIRECTIONS[0].center;
}
}

#undef NEXT
#undef PREV
#undef ENUM_NEXT
#undef ENUM_PREV