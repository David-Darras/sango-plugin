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

#ifdef __CLION_IDE__
#define USE_SANGO_PLUGIN
#endif

// Modular Core includes
#include "core/bitmask.h"
#include "core/color.h"
#include "core/math.h"
#include "core/memory.h"
#include "core/option.h"
#include "core/singleton.h"
#include "core/types.h"

#include <functional>
#include <CTRPluginFramework/System/Process.hpp>

// Address definitions
#include "address.h"

// Game constants
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"
#include "system/constant/language.h"

// Game structures shared by every domain
#include "core/native/bundle.h"
#include "pokemon/native/poke_info.h"
#include "system/native/message.h"
#include "system/native/string.h"

namespace ui {
class MainApplication;
}

typedef void (*menu_callback_t)(ui::MainApplication& app, void* args);
typedef std::function<void(void*)> callback_t;
typedef std::function<void()> cheat_code_callback_t;

