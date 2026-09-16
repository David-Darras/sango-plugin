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

#include "script/patch/native_script.h"
#include "overworld/patch/map_character.h"
#include "pokemon/constant/item.h"
#include "core/utils.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "script/constant/script.h"

namespace script {
namespace {
void LittlerootGreeter(Context& s) {
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
      auto item = core::Utils::GetRandomEnum<ItemId>();
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
  s.Face(kTalkTarget,
         (overworld::Facing)s.Call(s.GetNatives().PlayerGetReturnDir));

  s.Talk(u"See ya!");
  s.TalkEnd();
}

} // namespace

void Install() {
  NativeScript::Register(ScriptId::kLittlerootGreeter,
                         LittlerootGreeter);

  overworld::MapCharacterRequest greeter;
  greeter.map_id = MapId::kLittlerootTown;
  greeter.model_id = ModelId::kYoungster;
  greeter.script_id = ScriptId::kLittlerootGreeter;
  greeter.tile_x = 100;
  greeter.tile_z = 163;
  greeter.facing = overworld::Facing::kDown;
  overworld::MapCharacter::Add(greeter);
}
} // namespace script