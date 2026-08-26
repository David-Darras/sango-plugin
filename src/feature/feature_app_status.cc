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
#include "game/constant/nature.h"
#include "game/constant/species.h"
#include "game/global_data/item.h"
#include "game/global_data/pokemon.h"
#include "game/renderer/text_box.h"

#include "nature.inc"
#include "form.inc"

#define NEXT(x, val, max) x = ((x) + (val)) % (max)
#define PREV(x, val, max) x = ((x) - (val) + (max)) % (max)
#define ENUM_NEXT(type, x, val) x = static_cast<type>(( static_cast<u32>(x) + 1) % static_cast<u32>(type::kMax))
#define ENUM_PREV(type, x, val) x = static_cast<type>(( static_cast<u32>(x) - 1 + static_cast<u32>(type::kMax)) % static_cast<u32>(type::kMax))
// Maximum sum of a Pokemon's 6 EVs allowed by the games' mechanics.
static constexpr u32 kMaxTotalEv = 510;
#define DEFINE_PANE(color, id0, id1, name, field, max)\
static void next_##name(PokemonCoreData& core, u32 value) {\
NEXT(core.field, value, max);\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
PREV(core.field, value, max);\
}\
static Pane pane_##name(color, id0, id1, 0xFF, prev_##name, next_##name, nullptr);
#define DEFINE_STAT_PANE(str, name, id0, id1, id2, stat_field, iv_field, ev_field)\
static void next_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
NEXT(core.iv_field, value, 31 + 1);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
u32 x = core.ev_field;\
NEXT(core.ev_field, value, 252 + 1);\
if(core.GetTotalPower() > kMaxTotalEv) core.ev_field = x;\
}\
}\
static void prev_##name(PokemonCoreData& core, u32 value) {\
auto& ctx = feature::AppStatus::GetInstance();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
PREV(core.iv_field, value, 31 + 1);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
u32 x = core.ev_field;\
PREV(core.ev_field, value, 252 + 1);\
if(core.GetTotalPower() > kMaxTotalEv) core.ev_field = x;\
}\
}\
static void print_##name(Pane* pane, AppLayoutManager& manager, PokemonDataAccessor& accessor) {\
auto& ctx = feature::AppStatus::GetInstance();\
auto& core = *accessor.GetCoreData();\
auto& runtime = *accessor.GetRuntimeData();\
if (ctx.power_page_ == AppStatus::PowerPage::kIv) {\
pane->SetStringValue0(manager, u"IV %s", str);\
pane->SetIntegerValue(manager, (u32)core.iv_field, 2);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kEv) {\
pane->SetStringValue0(manager, u"EV %s", str);\
pane->SetIntegerValue(manager, (u32)core.ev_field, 3);\
} else if (ctx.power_page_ == AppStatus::PowerPage::kStat) {\
pane->SetStringValue0(manager, u"%s", str);\
pane->SetIntegerValue(manager, (u32)runtime.stat_field, 3);\
}\
}\
static Pane pane_##name(true, id0, id1, id2, prev_##name, next_##name, print_##name);

namespace feature {
class Pane {
public:
  using UpdatePokemonDataFunc = std::function<void(
      PokemonCoreData& core, u32 value)>;
  using PrintPokemonDataFunc = std::function<void(
      Pane* pane, AppLayoutManager& manager,
      PokemonDataAccessor& accessor)>;

  Pane(bool is_white, u8 id_0, u8 id_1, u8 id_2,
       UpdatePokemonDataFunc prev,
       UpdatePokemonDataFunc next,
       PrintPokemonDataFunc print) :
    is_white_(is_white),
    id_0_(id_0),
    id_1_(id_1),
    id_2_(id_2),
    prev_(prev),
    next_(next),
    print_(print) {
  }

  INLINE void Disable() { is_enabled_ = false; }
  INLINE bool IsEnabled() const { return is_enabled_; }

  INLINE bool IsWhite() const { return is_white_; }

  void Print(AppLayoutManager& manager, PokemonDataAccessor& accessor,
             Color8& top,
             Color8& bottom) {
    SetColors(manager, top, bottom, 1.0f);
    if (print_)
      print_(this, manager, accessor);
  }

  void SetStringValue0(AppLayoutManager& manager, const c16* str, ...) const {
    va_list args;
    va_start(args, str);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        String::GetTmpBuf(), BUFFER_SIZE, str, args);
    va_end(args);

    manager.SetTextBoxStringValue(0, id_0_, String::GetTmpBuf());
  }

  void SetStringValue1(AppLayoutManager& manager, const c16* str, ...) const {
    va_list args;
    va_start(args, str);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        String::GetTmpBuf(), BUFFER_SIZE, str, args);
    va_end(args);

    manager.SetTextBoxStringValue(0, id_1_, String::GetTmpBuf());
  }

  void SetIntegerValue(AppLayoutManager& manager, u32 value,
                       u32 count = 3) const {
    manager.SetTextBoxIntegerValue(0, id_1_, id_2_, value, count);
  }

  void SetColors(AppLayoutManager& manager, Color8& top, Color8& bottom,
                 f32 scale = 1.0f) const {
    manager.SetTextBoxColor(0, id_0_, top, bottom);
    renderer::Pane* left_pane = manager.GetPane(0, id_0_);
    if (left_pane != nullptr) {
      left_pane->scale = Vec2(scale, scale);
    }
    if (id_1_ != 0xFF) {
      manager.SetTextBoxColor(0, id_1_, top, bottom);
      renderer::Pane* right_pane = manager.GetPane(0, id_1_);
      if (right_pane != nullptr) {
        right_pane->scale = Vec2(scale, scale);
      }
    }
  }

  void Prev(PokemonCoreData& core, u32 value) { if (prev_) prev_(core, value); }
  void Next(PokemonCoreData& core, u32 value) { if (next_) next_(core, value); }

private:
  UpdatePokemonDataFunc prev_ = nullptr;
  UpdatePokemonDataFunc next_ = nullptr;
  PrintPokemonDataFunc print_ = nullptr;

  u8 id_0_ = 0;
  u8 id_1_ = 0;
  u8 id_2_ = 0;
  bool is_white_ = true;
  bool is_enabled_ = true;
};

DEFINE_PANE(false, 0, 0xFF, move_0, moves[0], MOVE_COUNT)
DEFINE_PANE(false, 18, 0xFF, move_1, moves[1], MOVE_COUNT)
DEFINE_PANE(false, 34, 0xFF, move_2, moves[2], MOVE_COUNT)
DEFINE_PANE(false, 50, 0xFF, move_3, moves[3], MOVE_COUNT)
DEFINE_PANE(false, 2, 0xFF, gender, gender, 3)
DEFINE_PANE(true, 8, 0xFF, item, item, ITEM_COUNT)
DEFINE_PANE(false, 0xFF, 0xFF, contest_cool, contest.cool, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_beautiful, contest.beautiful, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_cute, contest.cute, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_smart, contest.smart, 256)
DEFINE_PANE(false, 0xFF, 0xFF, contest_tough, contest.tough, 256)

DEFINE_STAT_PANE("HP", hp, 0, 4, 17, hp, iv_hp, ev_hp)
DEFINE_STAT_PANE("Attack", attack, 8, 10, 19, attack, iv_attack, ev_attack)
DEFINE_STAT_PANE("Defense", defense, 12, 14, 21, defense, iv_defense,
                 ev_defense)
DEFINE_STAT_PANE("Sp. Atk", sp_attack, 16, 18, 23, special_attack,
                 iv_special_attack,
                 ev_special_attack)
DEFINE_STAT_PANE("Sp. Def", sp_defense, 20, 22, 25, special_defense,
                 iv_special_defense,
                 ev_special_defense)
DEFINE_STAT_PANE("Speed", speed, 24, 26, 27, speed, iv_speed, ev_speed)

static void toggle_shiny(PokemonCoreData& core, u32 value) {
  if (PokemonUtils::IsShiny(core.id, core.shiny_id)) {
    PokemonUtils::ConvertToNormal(core.id, &core.shiny_id);
  } else {
    PokemonUtils::ConvertToShiny(core.id, &core.shiny_id);
  }
}

static Pane pane_shiny
    (true, 66, 68, 11, toggle_shiny, toggle_shiny,
     [](Pane* pane, AppLayoutManager& manager,
        PokemonDataAccessor& accessor) {
       pane->SetStringValue0(manager, u"Is Shiny");
       auto& core = *accessor.GetCoreData();
       u32 is_shiny = PokemonUtils::IsShiny(
           core.id, core.shiny_id);
       pane->SetStringValue1(manager, is_shiny ? u"Yes" : u"No");
     });

static Pane pane_level
    (true, 4, 6, 0xFF, [](PokemonCoreData& core, u32 value) {
       s8 level = PokemonUtils::GetLevelFromExperience(
           core.species, core.form, core.experience);
       level -= value;
       if (level < 1) level = 100;
       core.experience = PokemonUtils::GetExperienceFromLevel(
           core.species, core.form, level);
     }, [](PokemonCoreData& core, u32 value) {
       s8 level = PokemonUtils::GetLevelFromExperience(
           core.species, core.form, core.experience);
       level += value;
       if (level > 100) level = 1;
       core.experience = PokemonUtils::GetExperienceFromLevel(
           core.species, core.form, level);
     },
     nullptr);

static Pane pane_species
    (true, 58, 60, 1, [](PokemonCoreData& core, u32 value) {
       PREV(core.species, value, SPECIES_COUNT);
       core.form = 0;
     }, [](PokemonCoreData& core, u32 value) {
       NEXT(core.species, value, SPECIES_COUNT);
       core.form = 0;
     }, [](Pane* pane, AppLayoutManager& manager,
           PokemonDataAccessor& accessor) {
       auto& core = *accessor.GetCoreData();
       pane->SetIntegerValue(manager, accessor.GetCoreData()->species, 3);
     });

static Pane pane_item_ball
    (true, 8, 0xFF, 0xFF,
     [](PokemonCoreData& core, u32 value) {
       auto& ctx = AppStatus::GetInstance();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kHeldItem) {
         PREV(core.item, value, ITEM_COUNT);
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kBall) {
         PREV(core.ball, value, BALL_COUNT);
       }
     }, [](PokemonCoreData& core, u32 value) {
       auto& ctx = AppStatus::GetInstance();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kHeldItem) {
         NEXT(core.item, value, ITEM_COUNT);
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kBall) {
         NEXT(core.ball, value, BALL_COUNT);
       }
     }, [](Pane* pane, AppLayoutManager& manager,
           PokemonDataAccessor& accessor) {
       auto& core = *accessor.GetCoreData();
       auto& ctx = AppStatus::GetInstance();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kHeldItem) {
         /* CHANGE NOTHING */
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kBall) {
         u16 item_id =
             PokemonUtils::ConvertBallIdToItemId(
                 core.ball);
         global_data::Item item(item_id);
         item.GetName(String::GetTmpStr());
         pane->SetStringValue0(manager, String::GetTmpBuf());
       }
     });

static Pane pane_nature_form
    (true, 52, 50, 47,
     [](PokemonCoreData& core, u32 value) {
       auto& ctx = AppStatus::GetInstance();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kNature) {
         PREV(core.nature, value, NATURE_COUNT);
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kForm) {
         PREV(core.form, value,
              GetFormCount(core.species));
       }
     }, [](PokemonCoreData& core, u32 value) {
       auto& ctx = AppStatus::GetInstance();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kNature) {
         NEXT(core.nature, value, NATURE_COUNT);
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kForm) {
         NEXT(core.form, value,
              GetFormCount(core.species));
       }
     }, [](Pane* pane, AppLayoutManager& manager,
           PokemonDataAccessor& accessor) {
       auto& ctx = AppStatus::GetInstance();
       auto& core = *accessor.GetCoreData();
       if (ctx.item_page_ ==
           AppStatus::ItemPage::kNature) {
         pane->SetStringValue0(manager, u"Nature");
         pane->SetStringValue1(manager, u"%s",
                               NATURE_NAMES[core.nature]);
       } else if (
         ctx.item_page_ == AppStatus::ItemPage::kForm) {
         pane->SetStringValue0(manager, u"Form");
         pane->SetStringValue1(manager, u"%s",
                               GetFormName(core.species, core.form));
       }
     }
        );

static u8 ability_index = 0;
static Pane pane_ability
    (true, 38, 40, 0xFF,
     [](PokemonCoreData& core, u32 value) {
#ifdef KAIZO
       auto& pkm = global_data::Pokemon::GetInstance(core.species, core.form);
       PREV(ability_index, 1, 3);
       core.ability = pkm.ability[ability_index];
#else
       PREV(core.ability, value, ABILITY_COUNT);
#endif
     }, [](PokemonCoreData& core, u32 value) {
#ifdef KAIZO
       auto& pkm = global_data::Pokemon::GetInstance(core.species, core.form);
       NEXT(ability_index, 1, 3);
       core.ability = pkm.ability[ability_index];
#else
       NEXT(core.ability, value, ABILITY_COUNT);
#endif
     }, nullptr);


static Pane pane_null(true, 0xFF, 0xFF, 0xFF, nullptr, nullptr, nullptr);

void AppStatus::Update(savedata::PokemonParam& pokemon,
                       Controller& controller) {
  static Pane* FIRST_PANES[] = {
      &pane_species, &pane_contest_cool, &pane_null
  };

  const Key key = controller.GetRepeatedKey();
  bool refresh = false;
  auto& accessor = *pokemon.accessor;

  switch (mode_) {
    case Mode::kIdle: {
      switch (key) {
        case Key::kY:
          refresh = true;
          current_pane_ = FIRST_PANES[static_cast<u32>(sub_menu_)];
          ChangeMode(Mode::kNavigation);
          break;
        case Key::kR:
          refresh = true;
          ENUM_NEXT(PowerPage, power_page_, 1);
          ENUM_NEXT(ItemPage, item_page_, 1);
          break;
        case Key::kL:
          refresh = true;
          ENUM_PREV(PowerPage, power_page_, 1);
          ENUM_PREV(ItemPage, item_page_, 1);
          break;
        case Key::kRight:
          switch (sub_menu_) {
            case SubMenu::kParamsAndMoves:
              sub_menu_ = SubMenu::kContest;
              break;
            case SubMenu::kContest:
              sub_menu_ = SubMenu::kOther;
              break;
            default:
              break;
          }
          break;
        case Key::kLeft:
          switch (sub_menu_) {
            case SubMenu::kOther:
              sub_menu_ = SubMenu::kContest;
              break;
            case SubMenu::kContest:
              sub_menu_ = SubMenu::kParamsAndMoves;
              break;
            default:
              break;
          }
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
          if (current_pane_->IsEnabled()) {
            ChangeMode(Mode::kEditing);
          }
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
        case Key::kX:
          core.SetMaxContest();
          core.SetMaxIVs();
          core.SetStats(0, 0, 0, 0, 0, 0);
          break;
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
      // To fix PP
      pokemon.UpdateRuntimeData();
      savedata::PokemonTeam::GetInstance().HealAllPokemons();
      break;
    }
  }

  if (refresh && key != Key::kNone) {
    bool new_model = mode_ == Mode::kEditing &&
                     (IsOn(&pane_species) || IsOn(&pane_gender) ||
                      (IsOn(&pane_nature_form) && item_page_ == ItemPage::kForm)
                      || IsOn(&pane_shiny));
    // Reverse-engineered layout of the AppStatus process instance: at
    // +kOffsetToRenderContext sits a pointer to its render context, and
    // +kOffsetToUpdatePokemonFlag within that context is the bool that
    // tells the game to rebuild the on-screen Pokemon model.
    constexpr uptr kOffsetToRenderContext = 104;
    constexpr uptr kOffsetToUpdatePokemonFlag = 284;
    game::BaseProcess* process = game::ProcessManager::GetInstance().
        GetCurrentProcess();
    bool update_pokemon = READB(READ32((uptr)process + kOffsetToRenderContext) + kOffsetToUpdatePokemonFlag);
    if (!new_model) {
      WRITEB(READ32((uptr)process + kOffsetToRenderContext) + kOffsetToUpdatePokemonFlag, false);
    }
    ((void(*)(void*, u32, bool))ADDRESS_APP_STATUS_UPDATE_POKEMON)(process,
      GetSlot(), false);
    if (!new_model) {
      WRITEB(READ32((uptr)process + kOffsetToRenderContext) + kOffsetToUpdatePokemonFlag, true);
    }
  }
}

void AppStatus::MoveAcrossPanes(Controller& controller) {
  struct Direction {
    Pane* center;
    Pane* up;
    Pane* right;
    Pane* down;
    Pane* left;
  };

  static Direction DIRECTIONS[] = {

      {&pane_species, nullptr, &pane_hp, &pane_nature_form, nullptr},
      {&pane_nature_form, &pane_species, &pane_sp_attack, &pane_shiny, nullptr},
      {&pane_shiny, &pane_nature_form, &pane_speed, &pane_gender, nullptr},
      {&pane_gender, &pane_shiny, &pane_move_0, &pane_level, &pane_level},
      {&pane_level, &pane_gender, &pane_move_0, &pane_item_ball, nullptr},
      {&pane_item_ball, &pane_level, &pane_move_3, nullptr, nullptr},

      {&pane_hp, nullptr, nullptr, &pane_attack, &pane_species},
      {&pane_attack, &pane_hp, nullptr, &pane_defense, &pane_species},
      {&pane_defense, &pane_attack, nullptr, &pane_sp_attack, &pane_species},
      {&pane_sp_attack, &pane_defense, nullptr, &pane_sp_defense,
       &pane_species},
      {&pane_sp_defense, &pane_sp_attack, nullptr, &pane_speed, &pane_species},
      {&pane_speed, &pane_sp_defense, nullptr, &pane_ability, &pane_species},
      {&pane_ability, &pane_speed, nullptr, &pane_move_0, &pane_species},

      {&pane_move_0, &pane_ability, nullptr, &pane_move_1, &pane_level},
      {&pane_move_1, &pane_move_0, nullptr, &pane_move_2, &pane_level},
      {&pane_move_2, &pane_move_1, nullptr, &pane_move_3, &pane_item_ball},
      {&pane_move_3, &pane_move_2, nullptr, nullptr, &pane_item_ball},

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

      {&pane_null, nullptr, nullptr, nullptr, nullptr}
  };

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
  static Color8 WHITE(255, 255, 255, 255);
  static Color8 BLACK(0, 0, 0, 255);

  accessor.Decrypt();
  for (u32 i = 0; i < pane_count; i++) {
    if (panes[i]->IsWhite()) {
      panes[i]->Print(manager, accessor, WHITE, WHITE);
    } else {
      panes[i]->Print(manager, accessor, BLACK, BLACK);
    }
  }
  accessor.Encrypt();

  bool contains_current = false;
  for (u32 i = 0; i < pane_count; i++) {
    if (IsOn(panes[i])) {
      contains_current = true;
      break;
    }
  }

  if (contains_current) {
    if (mode_ == Mode::kNavigation) {
      if (current_pane_->IsWhite()) {
        Color8 top(255, 0, 0, 255);
        Color8 bottom(255, 255, 0, 255);
        current_pane_->SetColors(manager, top, bottom, 1.1f);
      } else {
        Color8 top(255, 0, 0, 255);
        Color8 bottom(120, 120, 120, 255);
        current_pane_->SetColors(manager, top, bottom, 1.1f);
      }
    } else if (mode_ == Mode::kEditing) {
      if (current_pane_->IsWhite()) {
        Color8 top(255, 0, 255, 255);
        Color8 bottom(255, 255, 255, 255);
        current_pane_->SetColors(manager, top, bottom, 1.1f);
      } else {
        Color8 top(255, 0, 255, 255);
        Color8 bottom(120, 120, 120, 255);
        current_pane_->SetColors(manager, top, bottom, 1.1f);
      }
    }
  }
}

void AppStatus::SetupGraphicsParamsHook(uptr self,
                                        savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsParams, self, pokemon);

  static Pane* PARAMS_PANES[] = {
      &pane_species, &pane_nature_form, &pane_shiny,
      &pane_hp, &pane_attack,
      &pane_defense, &pane_sp_attack, &pane_sp_defense,
      &pane_speed, &pane_ability
  };
  auto& manager = *(AppLayoutManager*)(READ32(self + 8 + 16));
  GetInstance().Draw(PARAMS_PANES, SIZE(PARAMS_PANES), *pokemon->accessor,
                     manager);

  manager.HidePane(0, 2); // don't show /
  manager.HidePane(0, 6); // don't show max hp
}

void AppStatus::SetupGraphicsMovesHook(uptr self,
                                       savedata::PokemonParam* pokemon,
                                       u8 move_index) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsMoves, self, pokemon,
                          move_index);

  static Pane* MOVES_PANES[] = {
      &pane_move_0, &pane_move_1, &pane_move_2, &pane_move_3
  };
  auto& manager = *(AppLayoutManager*)(READ32(self + 16));
  GetInstance().Draw(MOVES_PANES, SIZE(MOVES_PANES), *pokemon->accessor,
                     manager);
}

void AppStatus::SetupGraphicsInfosHook(uptr self,
                                       savedata::PokemonParam* pokemon) {
  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsInfos, self, pokemon);

  static Pane* INFOS_PANES[] = {
      &pane_level, &pane_gender, &pane_item_ball
  };
  auto& manager = *(AppLayoutManager*)(READ32(self + 8 + 16));
  GetInstance().Draw(INFOS_PANES, SIZE(INFOS_PANES), *pokemon->accessor,
                     manager);
}

void AppStatus::SetupGraphicsContestHook(uptr self,
                                         savedata::PokemonParam* pokemon) {
  static const u8 CONTEST_IDS[5][6] = {
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

  HookManager::Call<void>(HookID::kAppStatusSetupGraphicsContest, self,
                          pokemon);

  auto& ctx = GetInstance();
  auto& manager = *(AppLayoutManager*)(READ32(self + 8 + 16));

  for (u32 i = 0; i < 5; i++) {
    if (ctx.IsOn(CONTEST_PANES[i])) {
      manager.ShowPane(0, CONTEST_IDS[i][0]);
      manager.ShowPane(0, CONTEST_IDS[i][1]);
      manager.ShowPane(0, CONTEST_IDS[i][2]);
      manager.ShowAnimation(0, CONTEST_IDS[i][3]);
      manager.ShowAnimation(0, CONTEST_IDS[i][4]);
    } else {
      manager.HidePane(0, CONTEST_IDS[i][0]);
      manager.HidePane(0, CONTEST_IDS[i][1]);
      manager.HidePane(0, CONTEST_IDS[i][2]);
      manager.HideAnimation(0, CONTEST_IDS[i][3]);
      manager.HideAnimation(0, CONTEST_IDS[i][4]);
    }
  }
}

void AppStatus::PatchOnUpdate() {
  auto& pokemon = *savedata::PokemonTeam::GetInstance().pokemons[GetSlot()];
  auto& controller = Controller::GetInstance();
  GetInstance().Update(pokemon, controller);
}

void AppStatus::PatchOnLoad() {
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsParams);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsMoves);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsContest);
  HookManager::ForceEnable(HookID::kAppStatusSetupGraphicsInfos);
  GetInstance().Reset();
#ifdef KAIZO
  pane_level.Disable();
  pane_gender.Disable();
  pane_shiny.Disable();
  pane_move_0.Disable();
  pane_move_1.Disable();
  pane_move_2.Disable();
  pane_move_3.Disable();
  pane_species.Disable();
  pane_item_ball.Disable();
#endif
}
}

#undef NEXT
#undef PREV
#undef ENUM_NEXT
#undef ENUM_PREV
#undef DEFINE_PANE
#undef DEFINE_STAT_PANE