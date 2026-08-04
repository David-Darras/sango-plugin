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
#include "game/constant/move.h"
#include "game/constant/item.h"
#include "game/constant/ball.h"
#include "game/constant/species.h"
#include "game/global_data/item.h"
#include "game/renderer/text_box.h"

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
 *   - left/right : update power_page_index
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
  Pane(bool is_white, u8 label_id,
       void (*prev)(PokemonCoreData& core, u32 value),
       void (*next)(PokemonCoreData& core, u32 value),
       void (*print)(Pane* pane, AppLayoutManager& manager,
                     PokemonDataAccessor& accessor) =
           nullptr) :
    is_white_(is_white),
    label_pane_id_(label_id),
    value_pane_id_(label_id + 2),
    prev_(prev),
    next_(next), print_(print) {
  }

  Pane(bool is_white, u8 label_id, u8 value_id, u8 text_id,
       void (*prev)(PokemonCoreData& core, u32 value),
       void (*next)(PokemonCoreData& core, u32 value),
       void (*print)(Pane* pane, AppLayoutManager& manager,
                     PokemonDataAccessor& accessor) =
           nullptr) :
    Pane(is_white, label_id, prev, next, print) {
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
  bool is_white_;
};

static Color8 WHITE(255, 255, 255, 255);
static Color8 BLACK(0, 0, 0, 255);

#define DEFINE_PANE(color, id0, id1, name, field, max)\
static void next_##name(PokemonCoreData& core, u32 value) {\
NEXT(core.field, value, max);\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
PREV(core.field, value, max);\
}\
static Pane pane_##name(color, id0, id1, 0xFF, prev_##name, next_##name, nullptr);

DEFINE_PANE(true, 58, 60, dex_no, species, SPECIES_COUNT)
DEFINE_PANE(true, 62, 64, id_no, id, 0xFFFFFFFF)
DEFINE_PANE(true, 38, 40, ability, ability, ABILITY_COUNT)
DEFINE_PANE(false, 0, 0xFF, move_0, moves[0], MOVE_COUNT)
DEFINE_PANE(false, 18, 0xFF, move_1, moves[1], MOVE_COUNT)
DEFINE_PANE(false, 34, 0xFF, move_2, moves[2], MOVE_COUNT)
DEFINE_PANE(false, 50, 0xFF, move_3, moves[3], MOVE_COUNT)
DEFINE_PANE(false, 0xFF, 0xFF, contest_cool, contest.cool, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_beautiful, contest.beautiful, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_cute, contest.cute, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_smart, contest.smart, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_tough, contest.tough, 256)
//DEFINE_PANE(false, 0xFF, 0xFF, contest_sheen, contest.sheen, 256)
DEFINE_PANE(false, 2, 0xFF, gender, gender, 3);
DEFINE_PANE(true, 8, 0xFF, item, item, ITEM_COUNT);

#undef DEFINE_PANE

#define STAT_PANE_FUNCS(str, name, id0, id1, id2, stat_field, iv_field, ev_field)\
static void next_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
NEXT(core.iv_field, value, 31 + 1);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
NEXT(core.ev_field, value, 252 + 1);\
}\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
PREV(core.iv_field, value, 31 + 1);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
PREV(core.ev_field, value, 252 + 1);\
}\
}\
static void print_##name(Pane* pane, AppLayoutManager& manager, PokemonDataAccessor& accessor) {\
auto& ctx = feature::AppStatus::GetInstance();\
auto& core = *accessor.GetCoreData();\
auto& runtime = *accessor.GetRuntimeData();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"IV %s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)core.iv_field, 2);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"EV %s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)core.ev_field, 3);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kStat) {\
manager.SetTextBoxStringValue(0, pane->label_pane_id_, u"%s", str);\
manager.SetTextBoxIntegerValue(0, pane->value_pane_id_, pane->text_id_, (u32)runtime.stat_field, 3);\
}\
}\
static Pane pane_##name(true, id0, id1, id2, prev_##name, next_##name, print_##name);

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

static Pane pane_form_no(true, 34, 36, 14, prev_form_no, next_form_no,
                         print_form_no);

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

static Pane pane_shiny(true, 66, 68, 11, toggle_shiny, toggle_shiny,
                       print_shiny);

static void next_level(PokemonCoreData& core, u32 value) {
  s8 level = PokemonUtils::GetLevelFromExperience(
      core.species, core.form, core.experience);
  level++;
  if (level > 100) level = 1;
  core.experience = PokemonUtils::GetExperienceFromLevel(
      core.species, core.form, level);
}

static void prev_level(PokemonCoreData& core, u32 value) {
  s8 level = PokemonUtils::GetLevelFromExperience(
      core.species, core.form, core.experience);
  level--;
  if (level < 1) level = 100;
  core.experience = PokemonUtils::GetExperienceFromLevel(
      core.species, core.form, level);
}

static Pane pane_level(true, 4, 6, 0xFF, prev_level, next_level,
                       nullptr);

static void next_item_ball(PokemonCoreData& core, u32 value) {
  auto& ctx = AppStatus::GetInstance();
  if (ctx.item_page_ == AppStatus::ItemPage::kHeldItem) {
    NEXT(core.item, value, ITEM_COUNT);
  } else if (ctx.item_page_ == AppStatus::ItemPage::kBall) {
    NEXT(core.ball, value, BALL_COUNT);
  }
}

static void prev_item_ball(PokemonCoreData& core, u32 value) {
  auto& ctx = AppStatus::GetInstance();
  if (ctx.item_page_ == AppStatus::ItemPage::kHeldItem) {
    PREV(core.item, value, ITEM_COUNT);
  } else if (ctx.item_page_ == AppStatus::ItemPage::kBall) {
    PREV(core.ball, value, BALL_COUNT);
  }
}

static void print_item_ball(Pane* pane, AppLayoutManager& manager,
                            PokemonDataAccessor& accessor) {
  auto& core = *accessor.GetCoreData();
  auto& ctx = AppStatus::GetInstance();
  if (ctx.item_page_ == AppStatus::ItemPage::kHeldItem) {
    /* CHANGE NOTHING */
  } else if (ctx.item_page_ == AppStatus::ItemPage::kBall) {
    u16 item_id = PokemonUtils::ConvertBallIdToItemId(core.ball);
    global_data::Item item(item_id);
    item.GetName(String::GetTmpStr());
    manager.SetTextBoxStringValue(0, pane->label_pane_id_, String::GetTmpBuf());
  }
}

static Pane pane_item_ball(true, 8, 0xFF, 0xFF, prev_item_ball, next_item_ball,
                           print_item_ball);

struct Direction {
  Pane* center;
  Pane* up;
  Pane* right;
  Pane* down;
  Pane* left;
};

static Direction DIRECTIONS[] = {
    // top-left
    {&pane_dex_no, nullptr, &pane_hp, &pane_id_no, nullptr},
    {&pane_id_no, &pane_dex_no, &pane_hp, &pane_shiny, nullptr},
    {&pane_shiny, &pane_id_no, &pane_hp, &pane_form_no, nullptr},
    {&pane_form_no, &pane_shiny, &pane_hp, &pane_level, nullptr},
    {&pane_gender, &pane_form_no, &pane_move_0, &pane_level, &pane_level},
    {&pane_level, &pane_gender, &pane_gender, &pane_item_ball, nullptr},
    {&pane_item_ball, &pane_level, &pane_gender, nullptr, nullptr},
    // top-right
    {&pane_hp, nullptr, nullptr, &pane_attack, &pane_dex_no},
    {&pane_attack, &pane_hp, nullptr, &pane_defense, &pane_dex_no},
    {&pane_defense, &pane_attack, nullptr, &pane_sp_attack, &pane_dex_no},
    {&pane_sp_attack, &pane_defense, nullptr, &pane_sp_defense, &pane_dex_no},
    {&pane_sp_defense, &pane_sp_attack, nullptr, &pane_speed, &pane_dex_no},
    {&pane_speed, &pane_sp_defense, nullptr, &pane_ability, &pane_dex_no},
    {&pane_ability, &pane_speed, nullptr, &pane_move_0, &pane_dex_no},
    // bottom-right
    {&pane_move_0, &pane_ability, nullptr, &pane_move_1, &pane_gender},
    {&pane_move_1, &pane_move_0, nullptr, &pane_move_2, &pane_gender},
    {&pane_move_2, &pane_move_1, nullptr, &pane_move_3, &pane_gender},
    {&pane_move_3, &pane_move_2, nullptr, nullptr, &pane_gender},
    // star
    {&pane_contest_cool, nullptr, &pane_contest_beautiful, &pane_contest_cute,
     &pane_contest_tough},
    {&pane_contest_beautiful, &pane_contest_cool, nullptr, &pane_contest_cute,
     &pane_contest_tough},
    {&pane_contest_cute, &pane_contest_beautiful, &pane_contest_beautiful,
     nullptr, &pane_contest_smart},
    {&pane_contest_smart, &pane_contest_tough, &pane_contest_cute, nullptr,
     &pane_contest_tough},
    {&pane_contest_tough, &pane_contest_cool, &pane_contest_beautiful,
     &pane_contest_smart, nullptr},
    //{&pane_contest_sheen, nullptr, &pane_contest_cool, nullptr, nullptr},
};

static Pane* DRAW_PARAMS[] = {
    &pane_dex_no, &pane_id_no, &pane_shiny,
    &pane_form_no, &pane_hp, &pane_attack,
    &pane_defense, &pane_sp_attack, &pane_sp_defense,
    &pane_speed, &pane_ability
};

static Pane* DRAW_MOVES[] = {
    &pane_move_0, &pane_move_1, &pane_move_2, &pane_move_3
};

static Pane* DRAW_INFOS[] = {
    &pane_level, &pane_gender, &pane_item_ball
};

static Pane* SUB_MENUS[] = {
    &pane_dex_no, &pane_contest_cool, &pane_dex_no
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
        case Key::kRight:
          ENUM_NEXT(SubMenu, sub_menu_, 1);
          break;
        case Key::kLeft:
          ENUM_PREV(SubMenu, sub_menu_, 1);
          break;
        case Key::kY:
          refresh = true;
          current_pane_ = SUB_MENUS[static_cast<u32>(sub_menu_)];
          ui::LogApplication::Print(u"c=%p", current_pane_);
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
          ENUM_NEXT(PowerPage, power_page_, 1);
          ENUM_NEXT(ItemPage, item_page_, 1);
          break;
        case Key::kL:
          ENUM_PREV(PowerPage, power_page_, 1);
          ENUM_PREV(ItemPage, item_page_, 1);
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
      // To fix PP
      savedata::PokemonTeam::GetInstance().HealAllPokemons();
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

void AppStatus::Draw(Pane* panes[], u32 pane_count,
                     PokemonDataAccessor& accessor,
                     AppLayoutManager& manager) const {
  accessor.Decrypt();
  for (u32 i = 0; i < pane_count; i++) {
    if (panes[i]->is_white_) {
      panes[i]->Draw(manager, accessor, WHITE, WHITE);
    } else {
      panes[i]->Draw(manager, accessor, BLACK, BLACK);
    }
  }
  accessor.Encrypt();

  bool contains_current = false;
  for (u32 i = 0; i < pane_count; i++) {
    if (panes[i] == current_pane_) {
      contains_current = true;
      break;
    }
  }

  if (contains_current) {
    if (mode_ == Mode::kNavigation) {
      if (current_pane_->is_white_) {
        Color8 top(255, 0, 0, 255);
        Color8 bottom(255, 255, 0, 255);
        current_pane_->SetColors(manager, top, bottom);
      } else {
        Color8 top(255, 0, 0, 255);
        Color8 bottom(120, 120, 120, 255);
        current_pane_->SetColors(manager, top, bottom);
      }
    } else if (mode_ == Mode::kEditing) {
      if (current_pane_->is_white_) {
        Color8 top(255, 0, 255, 255);
        Color8 bottom(255, 255, 255, 255);
        current_pane_->SetColors(manager, top, bottom);
      } else {
        Color8 top(255, 0, 255, 255);
        Color8 bottom(120, 120, 120, 255);
        current_pane_->SetColors(manager, top, bottom);
      }
    }
  }
}

void AppStatus::SetupGraphicsParamsHook(uptr self,
                                        savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsParams, self, pokemon);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ(uptr, self + 8 + 16));

  ctx.Draw(DRAW_PARAMS, SIZE(DRAW_PARAMS), *pokemon->accessor, manager);

  manager.HidePane(0, 2); // don't show /
  manager.HidePane(0, 6); // don't show max hp
}

void AppStatus::SetupGraphicsMovesHook(uptr self,
                                       savedata::PokemonParam* pokemon,
                                       u8 move_index) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsMoves, self, pokemon,
                          move_index);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ(uptr, self + 16));

  ctx.Draw(DRAW_MOVES, SIZE(DRAW_MOVES), *pokemon->accessor, manager);
}

// class IconLoaderThread : public Thread {
// public:
//   IconLoaderThread(u8 item_id, u8 slot, renderer::Picture* picture) :
//     Thread(), item_id_(ITEM_MASTER_BALL), slot_(slot), picture_(picture) {
//     Start(14);
//   }
//
//   ~IconLoaderThread() override {
//   }
//
//   void Update() override {
//     uptr icon_manager = READ(vu32, 0x8072518);
//     // Load Icon Texture
//     ((void(*)(uptr, u8, u16))0x41E51C)(icon_manager, 0, item_id_);
//     svcSleepThread(1000000);
//     bool is_finished = ((bool(*)(uptr, u8))0x41E5C0)(
//         icon_manager, 0);
//     if (is_finished) {
//       // Apply Icon Texture
//       ((void(*)(uptr, u8, renderer::Picture*))
//         0x41E740)(icon_manager, 0, picture_);
//     }
//   }
//
// private:
//   u8 item_id_ = ITEM_MASTER_BALL;
//   u8 slot_ = 0;
//   renderer::Picture* picture_;
// };


void AppStatus::SetupGraphicsInfosHook(uptr self,
                                       savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsInfos, self, pokemon);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ(uptr, self + 8 + 16));

  ctx.Draw(DRAW_INFOS, SIZE(DRAW_INFOS), *pokemon->accessor, manager);

  pokemon->accessor->Decrypt();
  u32 item_id = ((u16(*)(u8))0x3B7300)(pokemon->core->ball);
  pokemon->accessor->Encrypt();

  // renderer::Picture* picture = manager.
  //     GetPicture(0, pane_ball_0.label_pane_id_);
  // uptr icon_manager = READ(vu32, 0x8072518);
  // // Load Icon Texture
  // ((void(*)(uptr, u8, u16))0x41E51C)(icon_manager, 0, item_id);
  // CTRPluginFramework::Sleep(CTRPluginFramework::Microseconds(1));
  // bool is_finished = ((bool(*)(uptr, u8))0x41E5C0)(
  //     icon_manager, 0);
  // if (is_finished) {
  //   // Apply Icon Texture
  //   ((void(*)(uptr, u8, renderer::Picture*))
  //     0x41E740)(icon_manager, 0, picture);
  // }
}

void AppStatus::SetupGraphicsContestHook(uptr self,
                                         savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsContest, self,
                          pokemon);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ(uptr, self + 8 + 16));

  static const u8 CONTEST[5][6] = {
      {148, 146, 144, 12, 17, 22},
      {152, 164, 166, 13, 18, 23},
      {154, 168, 170, 14, 19, 24},
      {156, 172, 174, 15, 20, 25},
      {158, 176, 178, 16, 21, 26},
  };

  static const Pane* CONTEST_PANES[] = {
      &pane_contest_cool,
      &pane_contest_beautiful,
      &pane_contest_cute,
      &pane_contest_smart,
      &pane_contest_tough,
  };

  for (u32 i = 0; i < 5; i++) {
    if (ctx.current_pane_ == CONTEST_PANES[i]) {
      manager.ShowPane(0, CONTEST[i][0]);
      manager.ShowPane(0, CONTEST[i][1]);
      manager.ShowPane(0, CONTEST[i][2]);
      manager.ShowAnimation(0, CONTEST[i][3]);
      manager.ShowAnimation(0, CONTEST[i][4]);
    } else {
      manager.HidePane(0, CONTEST[i][0]);
      manager.HidePane(0, CONTEST[i][1]);
      manager.HidePane(0, CONTEST[i][2]);
      manager.HideAnimation(0, CONTEST[i][3]);
      manager.HideAnimation(0, CONTEST[i][4]);
    }
  }
}

void AppStatus::PatchOnUpdate() {
  auto& ctx = GetInstance();
  auto& team = savedata::PokemonTeam::GetInstance();
  auto& pokemon = *team.pokemons[ctx.slot_];
  auto& controller = Controller::GetInstance();

  ctx.Update(pokemon, controller);
}

void AppStatus::PatchOnLoad() {
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsParams);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsMoves);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsContest);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsInfos);
  auto& ctx = GetInstance();
  ctx.power_page_ = PowerPage::kStat;
  ctx.item_page_ = ItemPage::kHeldItem;
  ctx.slot_ = 0;
  ctx.sub_menu_ = SubMenu::kParamsAndMoves;
  ctx.ChangeMode(Mode::kIdle);
}
}

#undef NEXT
#undef PREV
#undef ENUM_NEXT
#undef ENUM_PREV