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

#include "core/types.h"

namespace renderer {
struct H3dModel;
}

namespace battle {

struct Environment {
  uptr vtable;
  uptr _0[11];
  renderer::H3dModel* ground;
  uptr _1[4 + 4 + 1];
  renderer::H3dModel* landscape;
  void* _2;
  renderer::H3dModel* unknow_0;
  void* _3[2];
  renderer::H3dModel* unknow_1;
};

} // namespace battle
