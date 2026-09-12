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

#include "overlay/scripts.h"

#include "feature/core/feature_native_script.h"
#include "feature/overworld/feature_map_character.h"
#include "game/constant/item.h"
#include "game/constant/map.h"
#include "game/constant/model.h"
#include "game/constant/script.h"
#include "script/context.h"

namespace script {
namespace {

void LittlerootGreeter(Context& s) {
  {
    s.TalkStart();
    s.Talk(u"Hi! You just arrived in Littleroot Town?\n"
        u"This script is written in C++, not in Pawn!");
    s.ShowMessage(u"Do you want a random item for 1000 Pokédollars?");
    const bool wants_item = s.AskYesNo();
    const u32 amount = 1000;
    if (wants_item) {
      if (s.GetMoney() < amount) {
        s.Talk(u"You don't have enough money!");
      } else {
        ItemId item = Utils::GetRandomItemId();
        if (s.CanGiveItem(item)) {
          s.SubMoney(amount);
          s.GiveItem(item);
          s.PlayJingle(kJingleItem);
          s.Talk(u"Here, take good care of it!");
        } else {
          s.Talk(u"Oh, your bag is full...");
        }
      }
    } else {
      s.Talk(u"Suit yourself!");
    }

    s.Face(kTalkTarget, overworld::Facing::kLeft);
    s.Wait(10);
    s.Face(kTalkTarget, overworld::Facing::kRight);
    s.Wait(10);
    s.Face(kTalkTarget, (overworld::Facing)s.Call(s.GetNatives().PlayerGetReturnDir));

    s.Talk(u"See ya!");
    s.TalkEnd();
  }

  static bool first_time = true;

  s.TalkStart();

  if (first_time) {
    s.Talk(u"Hi! You just arrived in Littleroot Town?\n"
        u"This script is written in C++, not in Pawn!");
  }

  s.ShowMessage(u"Do you want a random item for 1000 Pokédollars?");
  const bool wants_item = s.AskYesNo();
  s.CloseMessage();

  if (wants_item) {
    if (s.GetMoney() < 1000) {
      s.Talk(u"You don't have enough money!");
    } else {
      auto item = Utils::GetRandomItemId();
      if (s.CanGiveItem(item)) {
        s.SubMoney(1000);
        s.GiveItem(item);
        s.PlayJingle(kJingleItem);
        s.Talk(u"Here, take good care of it!");
      } else {
        s.Talk(u"Oh, your bag is full...");
      }
    }
  } else {
    s.Talk(u"Suit yourself!");
  }

  if (first_time) {
    s.Face(kTalkTarget, overworld::Facing::kLeft);
    s.Wait(10);
    s.Face(kTalkTarget, overworld::Facing::kRight);
    s.Wait(10);
    s.Face(kTalkTarget, (overworld::Facing)s.Call(s.GetNatives().PlayerGetReturnDir));
    first_time = false;
  }

  s.Talk(u"See ya!");
  s.TalkEnd();
}
} // namespace

void Install() {
  feature::NativeScript::Register(ScriptId::kLittlerootGreeter,
                                  LittlerootGreeter);

  feature::MapCharacterRequest greeter;
  greeter.map_id = static_cast<u16>(MapId::kLittlerootTown);
  greeter.model_id = ModelId::kYoungster;
  greeter.script_id = ScriptId::kLittlerootGreeter;
  greeter.tile_x = 100;
  greeter.tile_z = 163;
  greeter.facing = overworld::Facing::kDown;
  feature::MapCharacter::Add(greeter);
}
} // namespace script