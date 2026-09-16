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

#include "core/native/game_manager.h"
#include "savedata/native/pss_data.h"

namespace savedata {
class PssManager {
  SINGLETON(PssManager)

public:
  STATIC_INLINE PssManager& GetInstance() {
    return core::GameManager::GetInstance().GetPssManager();
  }

  INLINE PssProfilePayload& GetMyProfile() {
    return self_data_.user_data.datagram.profile;
  }

private:
  PssData self_data_;
};
} // namespace savedata
