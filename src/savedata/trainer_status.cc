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

#include "savedata/trainer_status.h"

#include "menu/plugin_menu.h"

namespace savedata {

void TrainerStatus::LoadMenu(menu::PluginMenu& menu) {
  TrainerStatus& data = GetInstance();

  menu.Add("Visible Trainer ID (TID)", data.trainer_id)
      .Add("Secret Trainer ID (SID)", data.secret_id)
      .Add("Player Name", data.name, kPlayerNameLen)
      .Add("Nickname", data.nickname, kPlayerNameLen)
      .Add("Gender (0:M, 1:F)", data.gender)
      .Add("Game Version", data.game_version)
      .Add("Language", data.language)
      .Add("PSS Icon", data.pss_icon)
      .Add("Region", data.region)
      .Add("Latitude", data.latitude)
      .Add("Longitude", data.longitude)
      .Add("NEX ID", data.nex_id)
      .Add("Principal ID", data.principal_id)
      .Add("Current Console ID", data.current_console_id)
      .Add("Original Console ID", data.original_console_id)
      .Add("PSS ID", data.pss_id);

  menu.Add("PSS Message 1", data.pss_messages[0], kPssMessageLen)
      .Add("PSS Message 2", data.pss_messages[1], kPssMessageLen)
      .Add("PSS Message 3", data.pss_messages[2], kPssMessageLen)
      .Add("PSS Message 4", data.pss_messages[3], kPssMessageLen)
      .Add("PSS Message 5", data.pss_messages[4], kPssMessageLen)
      .Add("PSS Message 6", data.pss_messages[5], kPssMessageLen);

  menu.Add("Reject Friend Requests", &data.pss_flags, 0, 1)
      .Add("Reject Acquaintance Requests", &data.pss_flags, 1, 1)
      .Add("Reject Passersby Requests", &data.pss_flags, 2, 1)
      .Add("Reject Voice Chat", &data.pss_flags, 3, 1)
      .Add("Reject PR Video Exchange", &data.pss_flags, 4, 1);

  menu.Add("Mega Ring Obtained", &data.mega_flags, 0, 1)
      .Add("Mega Rayquaza Evolution Unlock", &data.mega_flags, 1, 1)
      .Add("COPPA Restriction", data.coppa_restriction);
}

}  // namespace savedata