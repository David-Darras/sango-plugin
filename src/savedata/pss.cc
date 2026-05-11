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

#include "savedata/pss.h"

namespace savedata {
#include "savedata/pss.inc"

void PssProfilePayload::LoadMenu(menu::PluginMenu& menu, void* args) {
  PssProfilePayload& profile = *(PssProfilePayload*)args;

  menu.Add("Name", profile.name, 13)
      .Add("Shout-out Message", profile.shoutout_message, 17)
      .Add("Principal Id", profile.principal_id)
      .Add("Local Friend Code", profile.local_friend_code)

      .Add("Icon For Friends", &profile.flags, 0, 8)
      .Add("Icon For Passerby", &profile.flags, 8, 8)
      .Add("Gender", &profile.flags, 16, 4)
      .Add("Has Accepted EULA", &profile.flags, 20, 1)
      .Add("Has Promotion Video", &profile.flags, 21, 1)
      .Add("Has Pokemon In Party", &profile.flags, 22, 1)
      .Add("Has Hall Of Fame Completed", &profile.flags, 23, 1)
      .Add("Rejects Friend Requests", &profile.flags, 24, 1)
      .Add("Rejects Acquaintance Requests", &profile.flags, 25, 1)
      .Add("Rejects Passerby Requests", &profile.flags, 26, 1)
      .Add("Rejects Voice Chat", &profile.flags, 27, 1)
      .Add("Parental: Photo Sharing Disabled", &profile.flags, 28, 1)
      .Add("Parental: Internet Disabled", &profile.flags, 29, 1)
      .Add("Parental: Friend Reg. Closed", &profile.flags, 30, 1)
      .Add("Parental: Streetpass Disabled", &profile.flags, 31, 1)

      .Add("Geographic Region Id", profile.geographic_region_id)
      .Add("Birth Month", profile.birth_month)
      .Add("Birth Day", profile.birth_day)
      .Add("Game Version", profile.game_version)
      .Add("Console Region", profile.console_region)

      .Add("Rejects Promo Video Requests", &profile.flags2, 0, 1)
      .Add("Has Shout-Out Message", &profile.flags2, 1, 1)
      .Add("Meets Trade Conditions", &profile.flags2, 2, 1)
      .Add("Rejects Lower Version Battles", &profile.flags2, 3, 1)
      .Add("Rejects Lower Version PR Videos", &profile.flags2, 4, 1)
      .Add("Icon For Friend 2 (ORAS)", &profile.flags2, 5, 8)
      .Add("Icon For Passerby 2 (ORAS)", &profile.flags2, 13, 8);
}

void PssGroup::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u32 choice = 0;
  PssGroup& grp = *(PssGroup*)args;
  menu.Add("Profile n°", choice)
      .Add("Profile Editor", PssProfilePayload::LoadMenu,
           &grp.user_data[choice].datagram.profile);
}
} // namespace savedata